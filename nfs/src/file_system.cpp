#include "file_system.hpp"
#include "settings.hpp"
#include "timer.hpp"
#include <algorithm>
#include <future>
using namespace nfs;

void FileSystem::addRootFile(NDS *rom, usz &i, const String &name, u32 &size, u32 offset, usz &siz, u8 *narcDat, usz &fileOffset, usz startFile) {

	auto &fso = fileSystem[i];
	auto &parent = fileSystem[0];

	fso.name = name;
	fso.index = i;
	fso.parent = 0;
	fso.resource = fileOffset - startFile;
	fso.buf = Buffer(size, (u8*)rom + offset);
	fso.reservedSize = ((offset + size + 511) & ~511) - offset;

	Buffer buf = fso.buf;

	try {

		fso.compressed = fso.buf;
		fso.buf = CompressionHelper::decompressBLZ(fso.compressed, rom);

		//We returned data from our "compressed" buffer. This means it wasn't actually compressed, it just has a bit of paddding
		//Unset compressed to avoid freeing this and copies

		if (fso.buf.add() >= fso.compressed.add() && fso.buf.add() < fso.compressed.end())
			fso.compressed = {};

	} CATCH(e) {
		fso.buf = buf;
		fso.compressed = {};
	}

	fso.filePtr1 = &size;
	fso.indexInFolder = parent.objects++;
	fso.fileHint = fso.folderHint = FileSystemObject::root;

	if (parent.fileHint == FileSystemObject::root)
		parent.fileHint = i;

	ResourceInfo inf = ResourceHelper::read(fso.buf.add(), fso.buf.size(), narcDat);
	siz += inf.size;

	++parent.files;

	++i;
	++fileOffset;
}

FileSystem::FileSystem() {}
FileSystem::FileSystem(NDS *rom) {

	#ifdef USE_TIMER
		oi::Timer t;
	#endif

	if (!rom)
		EXCEPTION("FileSystem couldn't be created from null rom");

	u8 *off = (u8*) rom;
	off += rom->ftable_off;

	Buffer fnt = { rom->ftable_len, (u8*)rom + rom->ftable_off };
	Buffer fpos = { rom->falloc_len, (u8*)rom + rom->falloc_off };

	struct FolderInfo {
		u32 offset;
		u16 firstFilePosition;
		u16 relation;
	};

	FolderInfo *folderArray = fnt.begin<FolderInfo>();
	FolderInfo &root = *folderArray;

	if ((root.relation & 0xF000) != 0)
		EXCEPTION("FileSystem Couldn't find root folder");

	usz rootFolders = root.relation;
	usz startFile = root.firstFilePosition;

	if (fnt.size() < sizeof(FolderInfo) * rootFolders)
		EXCEPTION("FileSystem Out of bounds exception");

	#ifdef USE_TIMER
		t.lap("Startup");
	#endif

	//Initialize all folders

	fileSystem.resize(rootFolders);

	for (usz i = 0; i < rootFolders; ++i) {

		fileSystem[i].index = i;
		fileSystem[i].resource = i == 0 ? FileSystemObject::root : FileSystemObject::folder;
		fileSystem[i].parent = i == 0 ? FileSystemObject::root : folderArray[i].relation & 0xFFF;
		fileSystem[i].fileHint = fileSystem[i].folderHint = FileSystemObject::root;

		if (i == 0)
			fileSystem[i].name = "/";
	}

	#ifdef USE_TIMER
		t.lap("Initialize folders");
	#endif

	//Initialize all files

	Buffer next = fnt.offset(rootFolders * sizeof(FolderInfo));
	usz current = 0;
	usz i = rootFolders;
	usz fileOffset = startFile;

	usz siz = 0, subfiles = 0, supported = 0;

	static constexpr usz maxResourceSize = ResourceHelper::getMaxResourceSize();
	u8 narcDat[maxResourceSize];

	List<usz> subs;

	while (next.size()) {

		u8 len = next.consume();

		bool isFolder = (len & 0x80) != 0;
		usz strLen = len & 0x7F;
		bool jump = len == 0;

		if (jump && !next.size())
			break;

		if (strLen == 0) {

			strLen = next.consume();

			isFolder = (strLen & 0x80) != 0;

			if (strLen == 0xFF)
				break;

			strLen = strLen & 0x7F;
		}

		String str = next.consumeString(strLen);

		if (jump)
			++current;

		usz dir = current;

		if (isFolder)
			dir = next.consume<u16>() & 0xFFF;

		if (isFolder) {

			FileSystemObject &fso = fileSystem[dir];
			FileSystemObject &parent = fileSystem[fso.parent];

			fso.name = (parent.isRoot() ? "" : parent.name + "/") + str;

			usz &parentCount = parent.objects;

			fso.indexInFolder = parentCount;

			if (parent.folderHint == FileSystemObject::root)
				parent.folderHint = fso.index;

			++parent.folders;
			++parentCount;
		}
		else {

			if (current == rootFolders)
				break;

			FileSystemObject &parent = fileSystem[current];
			usz &parentCount = parent.objects;

			FileSystemObject fso;
			fso.name = parent.name + "/" + str;
			fso.index = i;
			fso.parent = dir;
			fso.resource = fileOffset - startFile;
			fso.indexInFolder = parentCount;
			fso.folderHint = fso.fileHint = FileSystemObject::root;

			if (parent.fileHint == FileSystemObject::root)
				parent.fileHint = i;

			++parent.files;
			++parentCount;
			++i;

			u32 beg = fpos.at<u32>(fileOffset * 8);
			u32 end = fpos.at<u32>(fileOffset * 8 + 4);
			u32 fileSiz = end - beg;

			if (fileSiz) {

				fso.buf = { fileSiz, (u8*) rom + beg };

				//Handle decompression

				if (CompressionHelper::getDecompressionAllocation(fso.buf)) {

					fso.compressed = fso.buf;
					fso.buf = CompressionHelper::decompress(fso.compressed);

					//Compression failed

					if (fso.buf.add() == fso.compressed.add())
						fso.compressed = {};
				}

				//Handle when decompression flags are present but it doesn't allocate
				//(This means the data is already there, just with padding in front)

				else if(
					CompressionHelper::getCompressionType(
						fso.buf.add(), fso.buf.size()
					) != CompressionType::None
				)
					fso.buf = CompressionHelper::decompress(fso.buf);
			}

			fso.reservedSize = ((end + 511) & ~511) - beg;
			fso.filePtrs = fpos.add<u32>(fileOffset * 8);

			ResourceInfo inf = ResourceHelper::read(fso.buf.add(), fileSiz, narcDat);
			siz += inf.size;

			if (inf.magicNumber != NBUO_num)
				++supported;

			if (inf.magicNumber == ResourceHelper::getMagicNumber<NARC>()) {

				NARC &narc = *(NARC*) narcDat;
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

	usz filesAndFolders = fileSystem.size();

	buffer = Buffer::alloc(siz + subfiles * maxResourceSize);
	vec = List<ArchiveObject>(filesAndFolders - rootFolders + subfiles);
	fileSystem.resize(fileSystem.size() + subfiles + 4);

	#ifdef USE_TIMER
		t.lap("Resize buffer and fileSystem for subresources");
	#endif

	//Setup all resources and register all narcs

	List<usz> narcs;
	narcs.reserve(filesAndFolders - rootFolders);

	usz offset = 0;

	for (i = rootFolders; i < filesAndFolders; ++i) {

		FileSystemObject &fso = fileSystem[i];
		Buffer &buf = fso.buf;

		ArchiveObject &ao = vec[i - rootFolders];

		ao.comp = fso.compressed;
		ao.filePtrs = fso.filePtrs;

		ao.buf = buf;
		ao.info = ResourceHelper::read(buf.add(), buf.size(), buffer.add(offset));

		ao.name = fso.name;
		ao.position = buffer.add(offset);

		offset += ao.info.size;

		if (ao.info.magicNumber == ResourceHelper::getMagicNumber(NARC{}))
			narcs.push_back(i);
	}

	folders = rootFolders;
	files = fileSystem.size() - rootFolders;

	#ifdef USE_TIMER
		t.lap("Intialize resources");
	#endif

	//Spread out the work over as many cores as possible; since we're loading lots of NARCs

	//#define FORCE_SINGLE_THREAD		//(Debugging)

	#ifdef FORCE_SINGLE_THREAD
		u32 threadCount = 1;
	#else
		u32 threadCount = std::thread::hardware_concurrency();
	#endif


	List<std::future<usz>> threads(threadCount);

	//Object that specifies where we are reading and what

	struct ThreadedNARC {
		usz narcCount, narcId, fileId, fileCount, dirs;
		usz *narcs;
		ArchiveObject *start;
		FileSystemObject *file;
		Buffer dat;
	};

	usz perThread = narcs.size() / threadCount;
	usz perThreadr = narcs.size() % threadCount;
	usz fileStart = filesAndFolders - rootFolders;

	ThreadedNARC thread{};
	thread.fileId = filesAndFolders;
	thread.start = vec.data();
	thread.file = fileSystem.data();
	thread.dirs = rootFolders;

	//Setup all threads

	for (i = 0; i < threadCount; ++i) {
		
		thread.narcCount = perThread + (i < perThreadr ? 1 : 0);
		thread.narcs = narcs.data() + thread.narcId;
		thread.fileCount = 0;

		for (usz j = thread.narcId, k = thread.narcId + thread.narcCount; j < k; ++j)
			thread.fileCount += subs[j];

		thread.dat = Buffer(
			thread.fileCount * maxResourceSize, 
			buffer.add(siz + (thread.fileId - fileStart) * maxResourceSize)
		);

		//Function call for loading all NARCs
		threads[i] = std::move(std::async([rom](ThreadedNARC thread) -> usz {

			u8 narcDat[maxResourceSize];
			usz supported = 0;

			for (usz i = 0, j = thread.narcCount; i < j; ++i) {

				usz fileId = thread.narcs[i];
				FileSystemObject *obj = thread.file + fileId;

				ResourceHelper::read(obj->buf.add(), obj->buf.size(), narcDat);

				NARC &narc = *(NARC*) narcDat;

				try {

					Archive arc(narc, obj->name);

					obj->files = obj->objects = arc.size();
					obj->fileHint = thread.fileId;

					Buffer arcb = arc.getBuffer();
					std::memcpy(thread.dat.add(), arcb.add(), arcb.size());

					usz off = 0;

					for (usz k = 0; k < arc.size(); ++k) {

						ArchiveObject &ao = arc[k];

						FileSystemObject *file = thread.file + thread.fileId;

						file->index = thread.fileId;
						file->parent = obj->index;
						file->indexInFolder = k;
						file->resource = thread.fileId - thread.dirs;
						file->name = obj->name + "/" + ao.name;
						file->folderHint = file->fileHint = FileSystemObject::root;
						file->buf = ao.buf;
						file->compressed = ao.comp;

						Buffer ogBuffer = ao.compression() ? ao.comp : ao.buf;

						//We assume ownership, since the parent will go out of scope anyways
						//So we avoid an unnecessary copy
						//We make sure it's not cleaned up by 'unsetting' compression

						if (ao.compression())
							ao.comp = {};

						//

						usz fileStart = usz(ogBuffer.begin() - (u8*)rom);
						usz fileEnd = usz(ogBuffer.end() - (u8*)rom);

						file->reservedSize = ((fileEnd + 511) & ~511) - fileStart;
						file->filePtrs = ao.filePtrs;

						if (ao.info.magicNumber != NBUO_num)
							++supported;

						++thread.fileId;

						ArchiveObject *arco = thread.start + file->resource;
						arco->buf = file->buf;
						arco->comp = file->compressed;
						arco->name = ao.name;
						arco->position = thread.dat.add(off);
						arco->info = ao.info;
						arco->filePtrs = ao.filePtrs;

						off += ao.info.size;

						if (off > arcb.size())
							EXCEPTION("Invalid file offset");
					}

					thread.dat.addOffset(arcb.size());

				} CATCH(err) {
					std::printf("Couldn't read NARC file \"%s\"\n", obj->name.c_str());
				}
			}

			return supported;
		}, thread));

		thread.narcId += thread.narcCount;
		thread.fileId += thread.fileCount;
	}

	for (auto &f : threads)
		supported += f.get();

	#ifdef USE_TIMER
		t.lap("Intialize subresource threads");
	#endif

	i = fileSystem.size() - 4;

	addRootFile(rom, i, "arm9.bin", rom->arm9_size, rom->arm9_offset, siz, narcDat, fileOffset, startFile);
	addRootFile(rom, i, "arm7.bin", rom->arm7_size, rom->arm7_offset, siz, narcDat, fileOffset, startFile);
	addRootFile(rom, i, "overlay_arm9.bin", rom->arm9_olen, rom->arm9_ooff, siz, narcDat, fileOffset, startFile);
	addRootFile(rom, i, "overlay_arm7.bin", rom->arm7_olen, rom->arm7_ooff, siz, narcDat, fileOffset, startFile);

	#ifdef USE_TIMER
		t.lap("Initialize code binaries");
	#endif

	std::printf(
		"Loaded FileSystem with %zu files; %zu were supported (%zu%%)\n", 
		fileSystem.size() - rootFolders, 
		supported, 
		usz(round(f64(supported) / (fileSystem.size() - rootFolders) * 100))
	);

	supportedFiles = supported;

	#ifdef USE_TIMER
		t.print();
	#endif
}

FileSystem::FileSystem(const FileSystem &fs) { _copy(fs); }
FileSystem::FileSystem(FileSystem &&fs) { _move(std::move(fs)); }

FileSystem &FileSystem::operator=(const FileSystem &fs) { 
	clear();
	_copy(fs); 
	return *this; 
}

FileSystem &FileSystem::operator=(FileSystem &&fs) {
	clear();
	_move(std::move(fs));
	return *this;
}

void FileSystem::_move(FileSystem &&fs) {

	files = fs.files;
	folders = fs.folders;
	supportedFiles = fs.supportedFiles;

	Archive::_move(std::move(fs));

	fileSystem = std::move(fs.fileSystem);
	fs.fileSystem.clear();
}

void FileSystem::_copy(const FileSystem &fs) {

	fileSystem = fs.fileSystem;

	files = fs.files;
	folders = fs.folders;
	supportedFiles = fs.supportedFiles;

	Archive::_copy(fs);

	//Re-establish copy link

	for (FileSystemObject &fso : fileSystem) {

		if (!fso.isCompressed())
			continue;

		fso.buf = vec[fso.resource].buf;
	}
}

bool FileSystem_InParent(FileSystem&, FileSystemObject &fso, usz i, usz, FileSystemObject *parent, List<FileSystemObject*> *vec) {
	
	usz in = fso.isFolder() ? i : i + parent->folders;
	vec->operator[](in) = &fso;

	return false;
}

List<FileSystemObject*> FileSystem::operator[](FileSystemObject &fso) {

	List<FileSystemObject*> lvec(fso.objects);
	foreachInFolder(FileSystem_InParent, fso, &fso, &lvec);

	return lvec;
}

List<FileSystemObject*> FileSystem::traverse(FileSystemObject &root, bool includeDirs, bool includeSubfiles) {

	List<FileSystemObject*> fsov = (*this)[root];

	for (usz i = 0; i < fsov.size(); ++i) {

		FileSystemObject &fso = *fsov[i];

		if ((fso.isFile() && includeSubfiles) || (fso.isFolder() && includeDirs)) {
			List<FileSystemObject*> vfso = traverse(fso, includeDirs);
			fsov.insert(fsov.end(), vfso.begin(), vfso.end());
		}
	}

	return fsov;
}

FileSystemObject *FileSystem::operator[](const String &path) {

	auto it = std::find_if(
		fileSystem.begin(), fileSystem.end(), 
		[path](FileSystemObject &fso) -> bool { return path == fso.name; }
	);

	if (it == fileSystem.end())
		return nullptr;

	return &(*it);
}