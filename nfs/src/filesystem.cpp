#include "filesystem.h"
using namespace nfs;

FileSystem::FileSystem() {}
FileSystem::FileSystem(NDS *rom) {

	#ifdef USE_TIMER
	oi::Timer t;
	#endif

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
		throw std::exception("FileSystem Couldn't find root folder");

	u16 rootFolders = root.relation;
	u16 startFile = root.firstFilePosition;

	if (fnt.size < sizeof(FolderInfo) * rootFolders)
		throw std::exception("FileSystem Out of bounds exception");

	#ifdef USE_TIMER
		t.lap("Startup");
	#endif

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

	Buffer next = fnt.offset(rootFolders * sizeof(FolderInfo));
	u16 current = 0;
	u16 i = rootFolders;
	u16 fileOffset = startFile;

	u32 siz = 0, subfiles = 0;

	u8 narcDat[sizeof(NARC)];

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

			fso.files = fso.folders = 0;
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
			fso.files = fso.folders = 0;
			fso.indexInFolder = parentCount;
			fso.folderHint = fso.fileHint = u16_MAX;

			if (parent.fileHint == u32_MAX)
				parent.fileHint = i;

			++parent.files;
			++parentCount;
			++i;

			u32 &beg = *(u32*)(fpos.ptr + fileOffset * 8);
			u32 &end = *(u32*)(fpos.ptr + fileOffset * 8 + 4);
			u32 len = end - beg;

			fso.buf = { len, ((u8*)rom) + beg };

			u32 type = ResourceHelper::getType(fso.buf.ptr);
			siz += (u32) ResourceHelper::getSize(type);

			if (ResourceHelper::isType<NARC>(fso.buf.ptr)) {

				ResourceInfo inf = ResourceHelper::read(fso.buf.ptr, len, narcDat);

				NARC &narc = *(NARC*)narcDat;
				BTAF &btaf = narc.at<0>();

				subfiles += fso.files = btaf.files;
			}

			fileSystem.push_back(fso);
			++fileOffset;
		}

	}

	#ifdef USE_TIMER
		t.lap("Initialize files");
	#endif

	u32 filesAndFolders = fileSystem.size();
	
	/*		u32 maxResourceFiles = ResourceHelper::getMaxResourceSize();*/
	buffer = Buffer::alloc(siz /*+ subfiles * maxResourceFiles*/);
	vec = std::vector<ArchiveObject>(filesAndFolders - rootFolders /*+ subfiles*/);
	fileSystem.resize(fileSystem.size() /*+ subfiles*/);

	#ifdef USE_TIMER
		t.lap("Resize buffer and fileSystem for subresources");
	#endif

	//std::vector<u32> narcs;
	u32 offset = 0;

	for (u32 i = rootFolders; i < filesAndFolders; ++i) {

		FileSystemObject &fso = fileSystem[i];
		Buffer &buf = fso.buf;

		ArchiveObject &ao = vec[i - rootFolders];

		ao.info = ResourceHelper::read(buf.ptr, buf.size, buffer.ptr + offset);

		ao.name = fso.name;
		ao.position = buffer.ptr + offset;
		offset += (u32) ao.info.size;

		/*if (ao.info.magicNumber == ResourceHelper::getMagicNumber(NARC{}))
			narcs.push_back(i);*/
	}

	#ifdef USE_TIMER
		t.lap("Intialize resources");
	#endif

	#ifdef USE_TIMER
		t.lap("Intialize subresource threads");
	#endif

	#ifdef USE_TIMER
		t.lap("Intialize subresources");
	#endif

	#ifdef USE_TIMER
		t.stop();
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
}