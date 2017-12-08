#include "FileSystem.h"
using namespace nfs;

FileSystem::FileSystem(std::vector<FileSystemObject> _files, std::vector<GenericResourceBase*> resources, Buffer buf) : NArchive(resources, buf), files(_files), fileC(0), folderC(0){
	
	for (u32 i = 0; i < files.size(); ++i) 
		if (files[i].isFile())
			++fileC;
		else
			++folderC;
}

FileSystem::FileSystem() {}

const FileSystemObject &FileSystem::operator[](std::string str) const {

	const FileSystemObject *ptr = nullptr;
	for (u32 i = 0; i < files.size(); ++i)
		if (files[i].path == str || files[i].name == str) {
			ptr = &files[i];
			break;
		}

	if (ptr == nullptr)
		throw(std::exception(std::string("Couldn't find file with path \"").append(str).append("\"").c_str()));

	return *ptr;
}

std::vector<const FileSystemObject*> FileSystem::operator[](const FileSystemObject &fso) const {

	auto iter = std::find(files.begin(), files.end(), fso);

	std::vector<const FileSystemObject*> filesInFolder;

	if (iter == files.end()) {
		throw("Couldn't find file in file system");
		return filesInFolder;
	}

	u32 id = (u32)(iter - files.begin());

	for (u32 i = 0; i < files.size(); ++i)
		if (files[i].parent == id)
			filesInFolder.push_back(&files[i]);

	return filesInFolder;
}

std::vector<const FileSystemObject*> FileSystem::traverseFolder(const FileSystemObject &fso, bool includeDirs) const {

	auto iter = std::find(files.begin(), files.end(), fso);

	std::vector<const FileSystemObject*> filesInFolder;

	if (iter == files.end()) {
		throw("Couldn't find file in file system");
		return filesInFolder;
	}

	u32 id = (u32)(iter - files.begin());

	for (u32 i = 0; i < files.size(); ++i)
		if (files[i].isFile() || includeDirs)
			if (files[i].parent == id)
				filesInFolder.push_back(&files[i]);
			else {
				u32 parent = files[i].parent;
				while (parent != u32_MAX) {
					if (parent == id) {
						filesInFolder.push_back(&files[i]);
						break;
					}
					parent = files[parent].parent;
				}
			}

			return filesInFolder;
}

std::vector<FileSystemObject>::const_iterator FileSystem::begin() { return files.begin(); }
std::vector<FileSystemObject>::const_iterator FileSystem::end() { return files.end(); }
u32 FileSystem::getFileObjectCount() { return (u32)files.size(); }
u32 FileSystem::getFileCount() { return fileC; } 
u32 FileSystem::getFolderCount() { return folderC; }

const FileSystemObject &FileSystem::operator[](u32 i) {
	if (i >= files.size())
		throw(std::exception("Out of bounds"));
	return files[i];
}

template<class T> bool FileSystem::isFile(T t) {
	try {
		const FileSystemObject &obj = (*this)[t];
		return obj.isFile();
	}
	catch (std::exception e) {
		throw(std::exception(std::string("Couldn't get file (\"").append(e.what()).append("\")").c_str()));
		return false;
	}
}

template<class T> bool FileSystem::isFolder(T t) {
	try {
		const FileSystemObject &obj = (*this)[t];
		return obj.isFolder();
	}
	catch (std::exception e) {
		throw(std::exception(std::string("Couldn't get file (\"").append(e.what()).append("\")").c_str()));
		return false;
	}
}

template<class T> bool FileSystem::isRoot(T t) {
	try {
		const FileSystemObject &obj = (*this)[t];
		return obj.isRoot();
	}
	catch (std::exception e) {
		throw(std::exception(std::string("Couldn't get file (\"").append(e.what()).append("\")").c_str()));
		return false;
	}
}

FileSystemObject::~FileSystemObject() {}

bool FileSystemObject::isFolder() const { return resource >= u32_MAX - 1; }
bool FileSystemObject::isFile() const { return !isFolder(); }
bool FileSystemObject::isRoot() const { return resource == u32_MAX - 1; }
bool FileSystemObject::hasParent() const { return resource != u32_MAX - 1; }
bool FileSystemObject::operator==(const FileSystemObject &other) const {
	return path == other.path && index == other.index && parent == other.parent && resource == other.resource && name == other.name;
}

std::string FileSystemObject::getExtension() const {
	if (!isFile()) return "";

	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos || pos == path.size() - 1)
		return "";

	std::string ext = std::string(path.c_str() + pos + 1, path.size() - 1 - pos);
	for (u32 i = 0; i < ext.size(); ++i)
		ext[i] = toupper(ext[i]);
	return ext;
}

bool FileSystemObject::getMagicNumber(std::string &name, u32 &number) const {

	if (!isFile()) {
		name = "";
		number = 0;
		return true;
	}

	u32 magicNumber = 0;
	std::string extension = getExtension(), extensionReverse = extension, type = extension;
	std::reverse(extensionReverse.begin(), extensionReverse.end());

	magicNumber = extension.size() == 4 ? *(u32*)extension.c_str() : 0;

	bool valid = false;

	try {
		runArchiveFunction<NType::IsValidType>(magicNumber, ArchiveTypes(), &valid);
	}
	catch (std::exception e) {}

	if (!valid && extension != "TXT" && extension != "BIN" && extension != "DAT") {
		type = std::string((char*)buffer.data, 4);
		valid = true;

		if (type != extensionReverse) {

			for (u32 i = 0; i < 4; ++i)
				if (!((type[i] >= 'A' && type[i] <= 'Z') || type[i] == '0'))
					valid = false;

			if (valid) 
				magicNumber = *(u32*)type.c_str();
			else
				type = extension;
		}
		else {
			magicNumber = *(u32*)type.c_str();
			type = extension;
		}
	}

	name = type;
	number = magicNumber;

	valid = false;

	try {
		runArchiveFunction<NType::IsValidType>(magicNumber, ArchiveTypes(), &valid);
	} catch (std::exception e) {}

	return valid;
}

bool NType::convert(NDS nds, FileSystem *fs) {

	Buffer fileNames = offset(nds.data, nds.ftable_off - nds.romHeaderSize);
	fileNames.size = nds.ftable_len;

	Buffer filePositions = offset(nds.data, nds.falloc_off - nds.romHeaderSize);
	filePositions.size = nds.falloc_len;

	if (fileNames.size < sizeof(FolderInfo)) {
		throw(std::exception("Invalid buffer size"));
		return false;
	}

	FolderInfo *folderArray = (FolderInfo*)fileNames.data;
	FolderInfo &root = folderArray[0];

	if ((root.relation & 0xF000) != 0) {
		throw(std::exception("Invalid file table; first folder isn't root"));
		return false;
	}

	u32 folderArraySize = root.relation;
	u32 startFile = root.firstFilePosition;

	if (fileNames.size < sizeof(FolderInfo) * folderArraySize) {
		throw(std::exception("Out of bounds exception"));
		return false;
	}

	std::vector<FileSystemObject> fso(folderArraySize);
	fso.reserve(folderArraySize + 512);

	for (u32 i = 0; i < folderArraySize; ++i) {
		fso[i].index = i;
		fso[i].resource = i == 0 ? u32_MAX - 1 : u32_MAX;
		fso[i].parent = i == 0 ? u32_MAX : folderArray[i].relation & 0xFFF;
		fso[i].fileOff = folderArray[i].firstFilePosition - startFile;

		if (i == 0)
			fso[i].path = fso[i].name = "/";
	}

	Buffer next = offset(fileNames, folderArraySize * sizeof(FolderInfo));

	u32 current = 0;
	u32 i = folderArraySize;
	u32 fileOffset = startFile;

	u32 bufferSize = 0;

	while (next.size > 0) {

		u32 curr = 0;
		u8 len = next.data[curr];
		++curr;

		bool isFolder = (len & 0x80) != 0;
		u32 strLen = len & 0x7F;
		bool jump = len == 0;

		if (strLen == 0) {
			strLen = next.data[curr];
			isFolder = (strLen & 0x80) != 0;

			if (strLen == 0xFF)
				break;

			strLen = strLen & 0x7F;

			++curr;
		}

		std::string str;

		if (strLen != 0) {
			str = std::string((char*)&next.data[curr], strLen);
			curr += strLen;
		}

		if (jump)
			++current;

		u16 dir = current;

		if (isFolder) {
			dir = (*(u16*)&next.data[curr]) & 0xFFF;
			curr += 2;
		}

		if (isFolder) {
			fso[dir].name = str;
			fso[dir].path = (fso[fso[dir].parent].isRoot() ? "" : (fso[fso[dir].parent].path + "/")) + str;
			fso[dir].buffer = { nullptr, 0 };
		}
		else {

			FileSystemObject fo;
			fo.name = str;
			fo.path = fso[current].path + "/" + str;
			fo.index = i;
			fo.parent = dir;
			fo.resource = fileOffset - startFile;
			++i;

			u32 &x = *(u32*)(filePositions.data + fileOffset * 8);
			u32 &y = *(u32*)(filePositions.data + fileOffset * 8 + 4);
			u32 len = y - x;

			fo.buffer = offset(nds.data, x - nds.romHeaderSize);
			fo.buffer.size = len;

			fso.push_back(fo);

			std::string name;
			u32 magicNumber;
			bool valid = fo.getMagicNumber(name, magicNumber);

			if (!valid)
				magicNumber = 0;

			runArchiveFunction<GenericResourceSize>(magicNumber, ArchiveTypes(), &bufferSize);

			++fileOffset;
		}

		next = offset(next, curr);
	}

	u32 totalFiles = (u32)fso.size() - folderArraySize;
	Buffer resources = newBuffer1(bufferSize);
	std::vector<GenericResourceBase*> resourcePtrs(totalFiles);
	u32 bufferOffset = 0;

	for (u32 i = folderArraySize; i < fso.size(); ++i) {

		const FileSystemObject &fo = fso[i];

		std::string name;
		u32 magicNumber;
		bool valid = fo.getMagicNumber(name, magicNumber);

		if (!valid)
			magicNumber = 0;

		u8 *at = resources.data + bufferOffset;
		u32 j = i - folderArraySize;

		resourcePtrs[j] = (GenericResourceBase*)at;

		u32 mlen = 0;
		runArchiveFunction<GenericResourceSize>(magicNumber, ArchiveTypes(), &mlen);

		try {
			runArchiveFunction<NFactory>(magicNumber, ArchiveTypes(), (void*)at, fo.buffer);
		}
		catch (std::exception e) {

			if (mlen >= sizeof(NBUO)) {
				runArchiveFunction<NFactory>(0, ArchiveTypes(), (void*)at, fo.buffer);
			}
			else
				memset(at, 0, mlen);
		}

		bufferOffset += mlen;
	}

	*fs = FileSystem(fso, resourcePtrs, resources);
	return true;
}