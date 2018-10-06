#include "filesystem.h"
#include "settings.h"
#include "timer.h"
#include <algorithm>
#include <future>
using namespace nfs;

FileSystem::FileSystem() {}
FileSystem::FileSystem(NDS *rom) {

	#ifdef USE_TIMER
	oi::Timer t;
	#endif

	if (rom == nullptr)
		EXCEPTION("FileSystem Couldn't be created from null rom");

	u8 *off = (u8*)rom;
	off += rom->ftable_off;

	Buffer fnt = { rom->ftable_len, ((u8*)rom) + rom->ftable_off };
	Buffer fpos = { rom->falloc_len, ((u8*)rom) + rom->falloc_off };

	struct FolderInfo {
		u32 offset;
		u16 firstFilePosition;
		u16 relation;
	};

	FolderInfo *folderArray = (FolderInfo*)fnt.ptr;
	FolderInfo &root = *folderArray;

	if ((root.relation & 0xF000) != 0)
		EXCEPTION("FileSystem Couldn't find root folder");

	u16 rootFolders = root.relation;
	u16 startFile = root.firstFilePosition;

	if (fnt.size < sizeof(FolderInfo) * rootFolders)
		EXCEPTION("FileSystem Out of bounds exception");

	#ifdef USE_TIMER
		t.lap("Startup");
	#endif

	//Initialize all folders

	fileSystem.resize(rootFolders);

	for (u16 i = 0; i < rootFolders; ++i) {
		fileSystem[i].index = i;
		fileSystem[i].resource = i == 0 ? u16_MAX : u16_MAX - 1;
		fileSystem[i].parent = i == 0 ? u16_MAX : folderArray[i].relation & 0xFFF;
		fileSystem[i].fileHint = fileSystem[i].folderHint = u16_MAX;

		if (i == 0)
			fileSystem[i].name = "/";
	}

	#ifdef USE_TIMER
		t.lap("Initialize folders");
	#endif

	//Initialize all files

	Buffer next = fnt.offset(rootFolders * sizeof(FolderInfo));
	u16 current = 0;
	u16 i = rootFolders;
	u16 fileOffset = startFile;

	u32 siz = 0, subfiles = 0, supported = 0;

	u8 narcDat[sizeof(NARC)];

	std::vector<u32> subs;

	while (next.size > 0) {

		u8 len = next[0];
		next.addOffset(1U);

		bool isFolder = (len & 0x80) != 0;
		u32 strLen = len & 0x7FU;
		bool jump = len == 0;

		if (strLen == 0) {

			strLen = next[0];

			next.addOffset(1U);
			isFolder = (strLen & 0x80) != 0;

			if (strLen == 0xFF)
				break;

			strLen = strLen & 0x7F;
		}

		std::string str((char*)next.ptr, strLen);
		next.addOffset(strLen);

		if (jump)
			++current;

		u16 dir = current;

		if (isFolder) {
			dir = (*(u16*)&next[0]) & 0xFFF;
			next.addOffset(2U);
		}

		if (isFolder) {

			FileSystemObject &fso = fileSystem[dir];
			FileSystemObject &parent = fileSystem[fso.parent];

			fso.name = (parent.isRoot() ? "" : parent.name + "/") + str;

			u16 &parentCount = parent.objects;

			fso.indexInFolder = parentCount;

			if (parent.folderHint == u16_MAX)
				parent.folderHint = fso.index;

			++parent.folders;
			++parentCount;

		}
		else {

			if (current == rootFolders)
				break;

			FileSystemObject &parent = fileSystem[current];
			u16 &parentCount = parent.objects;

			FileSystemObject fso;
			fso.name = parent.name + "/" + str;
			fso.index = i;
			fso.parent = dir;
			fso.resource = fileOffset - startFile;
			fso.indexInFolder = parentCount;
			fso.folderHint = fso.fileHint = u16_MAX;

			if (parent.fileHint == u16_MAX)
				parent.fileHint = i;

			++parent.files;
			++parentCount;
			++i;

			u32 beg = *(u32*)(fpos.ptr + fileOffset * 8);
			u32 end = *(u32*)(fpos.ptr + fileOffset * 8 + 4);
			u32 len = end - beg;

			fso.buf = { len, ((u8*)rom) + beg };

			u32 type = ResourceHelper::getType(fso.buf.ptr);
			siz += (u32) ResourceHelper::getSize(type);

			ResourceInfo inf = ResourceHelper::read(fso.buf.ptr, len, narcDat);

			if (inf.magicNumber != NBUO_num)
				++supported;

			if (inf.magicNumber == ResourceHelper::getMagicNumber<NARC>()) {

				NARC &narc = *(NARC*)narcDat;
				BTAF &btaf = narc.at<0>();

				subs.push_back(btaf.files);

				subfiles += fso.files = btaf.files;
			}

			fileSystem.push_back(fso);
			++fileOffset;
		}

	}

	#ifdef USE_TIMER
		t.lap("Initialize files");
	#endif

	//Resize buffers to support sub resources

	u32 filesAndFolders = fileSystem.size();
	
	constexpr u32 maxResourceFiles = ResourceHelper::getMaxResourceSize();
	buffer = Buffer::alloc(siz + subfiles * maxResourceFiles);
	vec = std::vector<ArchiveObject>(filesAndFolders - rootFolders + subfiles);
	fileSystem.resize(fileSystem.size() + subfiles);

	#ifdef USE_TIMER
		t.lap("Resize buffer and fileSystem for subresources");
	#endif

	//Setup all resources and register all narcs

	std::vector<u32> narcs;
	narcs.reserve(filesAndFolders - rootFolders);

	u32 offset = 0;

	for (u32 i = rootFolders; i < filesAndFolders; ++i) {

		FileSystemObject &fso = fileSystem[i];
		Buffer &buf = fso.buf;

		ArchiveObject &ao = vec[i - rootFolders];

		ao.buf = buf;
		ao.info = ResourceHelper::read(buf.ptr, buf.size, buffer.ptr + offset);

		ao.name = fso.name;
		ao.position = buffer.ptr + offset;
		offset += (u32) ao.info.size;

		if (ao.info.magicNumber == ResourceHelper::getMagicNumber(NARC{}))
			narcs.push_back(i);
	}

	folders = rootFolders;
	files = fileSystem.size() - rootFolders;

	#ifdef USE_TIMER
		t.lap("Intialize resources");
	#endif

	//Spread out the work over as many cores as possible; since we're loading lots of NARCs

	u32 threadCount = std::thread::hardware_concurrency();
	std::vector<std::future<u32>> threads(threadCount);

	//Object that specifies where we are reading and what

	struct ThreadedNARC {
		u32 narcCount, narcId, fileId, fileCount, dirs;
		u32 *narcs;
		ArchiveObject *start;
		FileSystemObject *file;
		Buffer dat;
	};

	u32 perThread = (u32)narcs.size() / threadCount;
	u32 perThreadr = (u32)narcs.size() % threadCount;
	u32 fileStart = filesAndFolders - rootFolders;

	ThreadedNARC thread;
	memset(&thread, 0, sizeof(thread));
	thread.fileId = filesAndFolders;
	thread.start = vec.data();
	thread.file = fileSystem.data();
	thread.dirs = rootFolders;

	//Setup all threads

	for (u32 i = 0; i < threadCount; ++i) {
		
		thread.narcCount = perThread + (i < perThreadr ? 1 : 0);
		thread.narcs = narcs.data() + thread.narcId;
		thread.fileCount = 0;

		for (u32 j = thread.narcId, k = thread.narcId + thread.narcCount; j < k; ++j)
			thread.fileCount += subs[j];

		thread.dat = Buffer(thread.fileCount * maxResourceFiles, buffer.ptr + siz + (thread.fileId - fileStart) * maxResourceFiles);

		threads[i] = std::move(std::async([](ThreadedNARC thread) -> u32 {	//Function call for loading all NARCs

			u8 narcDat[sizeof(NARC)];
			u8 resDat[ResourceHelper::getMaxResourceSize()];
			u32 supported = 0;

			for (u32 i = 0, j = thread.narcCount; i < j; ++i) {

				u32 fileId = thread.narcs[i];
				FileSystemObject *obj = thread.file + fileId;

				ResourceHelper::read(obj->buf.ptr, obj->buf.size, narcDat);

				NARC &narc = *(NARC*)narcDat;

				try {

					Archive arc(narc);

					obj->files = obj->objects = arc.size();
					obj->fileHint = thread.fileId;

					Buffer arcb = arc.getBuffer();
					memcpy(thread.dat.ptr, arcb.ptr, arcb.size);

					u32 off = 0;

					for (u32 k = 0; k < arc.size(); ++k) {

						ArchiveObject &ao = arc[k];

						FileSystemObject *file = thread.file + thread.fileId;

						file->index = thread.fileId;
						file->parent = obj->index;
						file->indexInFolder = k;
						file->resource = thread.fileId - thread.dirs;
						file->name = obj->name + "/" + ao.name;
						file->folderHint = file->fileHint = u16_MAX;
						file->buf = ao.buf;

						if (ao.info.magicNumber != NBUO_num)
							++supported;

						++thread.fileId;

						ArchiveObject *arco = thread.start + file->resource;
						arco->buf = ao.buf;
						arco->name = ao.name;
						arco->position = thread.dat.ptr + off;
						arco->info = ResourceHelper::read(ao.buf.ptr, ao.buf.size, resDat);

						off += arco->info.size;
					}

					thread.dat.addOffset(arcb.size);

				} catch (std::runtime_error err) {
					printf("Couldn't read NARC file \"%s\"\n", obj->name.c_str());
				}


			}

			return supported;

		}, thread));

		thread.narcId += thread.narcCount;
		thread.fileId += thread.fileCount;
	}

	for (auto &f : threads)
		supported += f.get();

	printf("Loaded FileSystem with %u files; %u were supported (%u%%)\n", (u32)(fileSystem.size() - rootFolders), supported, (u32)round((f32)supported / (fileSystem.size() - rootFolders) * 100));

	#ifdef USE_TIMER
		t.lap("Intialize subresource threads");
	#endif

	#ifdef USE_TIMER
		t.print();
	#endif
}

FileSystem::FileSystem(const FileSystem &fs) { copy(fs); }
FileSystem &FileSystem::operator=(const FileSystem &fs) { copy(fs); return *this; }

FileSystem::FileSystem(FileSystem &&fs) {
	fileSystem = std::move(fs.fileSystem);
	fs.fileSystem.clear();
}

std::vector<FileSystemObject> &FileSystem::getFileSystem() { return fileSystem; }

void FileSystem::copy(const FileSystem &fs) {
	fileSystem = fs.fileSystem;
	files = fs.files;
	folders = fs.folders;
	Archive::copy(fs);
}

std::vector<FileSystemObject>::iterator FileSystem::begin() { return fileSystem.begin(); }
std::vector<FileSystemObject>::iterator FileSystem::end() { return fileSystem.end(); }

FileSystemObject &FileSystem::operator[](u32 i) {
	return fileSystem[i];
}

ArchiveObject &FileSystem::getResource(FileSystemObject &fso) {
	return Archive::operator[](fso.resource);
}


bool FileSystem_InParent(FileSystem &fs, FileSystemObject &fso, u32 i, u32 index, FileSystemObject *parent, std::vector<FileSystemObject*> *vec) {
	
	u32 in = fso.isFolder() ? i : i + parent->folders;
	vec->operator[](in) = &fso;

	return false;
}

std::vector<FileSystemObject*> FileSystem::operator[](FileSystemObject &fso) {

	std::vector<FileSystemObject*> vec(fso.objects);
	foreachInFolder(FileSystem_InParent, fso, &fso, &vec);

	return vec;
}

std::vector<FileSystemObject*> FileSystem::traverse(FileSystemObject &root, bool includeDirs, bool includeSubfiles) {

	std::vector<FileSystemObject*> fsov = (*this)[root];

	for (u32 i = 0; i < fsov.size(); ++i) {

		FileSystemObject &fso = *fsov[i];

		if ((fso.isFile() && includeSubfiles) || (fso.isFolder() && includeDirs)) {
			std::vector<FileSystemObject*> vfso = traverse(fso, includeDirs);
			fsov.insert(fsov.end(), vfso.begin(), vfso.end());
		}
	}

	return fsov;
}

u32 FileSystem::size() const { return fileSystem.size(); }
u32 FileSystem::getFiles() const { return files; }
u32 FileSystem::getFolders() const { return folders; }

void FileSystem::clear() {
	fileSystem.clear();
	Archive::clear();
}

FileSystemObject *FileSystem::operator[](std::string path) {

	auto it = std::find_if(fileSystem.begin(), fileSystem.end(), [path](FileSystemObject &fso) -> bool { return path == fso.name; });

	if (it == fileSystem.end()) {
		EXCEPTION(("FileSystem Couldn't find file with path " + path).c_str());
		return nullptr;
	}

	return &(*it);
}