#include "FileSystem.h"
using namespace nfs;

FileSystem::FileSystem(std::vector<FileSystemObject> _files, std::vector<GenericResourceBase*> resources, Buffer buf) : NArchieve(resources, buf), files(_files) {}
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