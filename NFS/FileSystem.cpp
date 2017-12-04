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

	u32 id = (iter - files.begin());

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

	u32 id = (iter - files.begin());

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
u32 FileSystem::getFileObjectCount() { return files.size(); }
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