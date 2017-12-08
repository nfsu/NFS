#pragma once

#include "NTypes2.h"

namespace nfs {

	//An object for storing file info 
	struct FileSystemObject {

		std::string path, name;
		u32 index, parent, resource, fileOff;
		Buffer buffer;

		virtual ~FileSystemObject();

		bool isFolder() const;
		bool isFile() const;
		bool isRoot() const;
		bool hasParent() const;
		bool operator==(const FileSystemObject &other) const;
		std::string getExtension() const;
		bool getMagicNumber(std::string &name, u32 &number) const;
	};

	//A bundle of files; different than an archieve
	//An archieve is a list of files, while this can also contain folders
	class FileSystem : public NArchive {

	public:

		FileSystem(std::vector<FileSystemObject> _files, std::vector<GenericResourceBase*> resources, Buffer buf);
		FileSystem();

		template<class T>
		const T &getResource(std::string str) const;

		template<class T>
		const T &getResource(const FileSystemObject &fso) const;
		
		const FileSystemObject &operator[](std::string str) const;

		//Gets all files and folders in folder
		//For getting all files inside of those folders, use traverseFolder(fso)
		std::vector<const FileSystemObject*> operator[](const FileSystemObject &fso) const;

		//Find all files in folder
		//bool includeDirs = false
		//Turn includeDirs to true if you want to include folders
		std::vector<const FileSystemObject*> traverseFolder(const FileSystemObject &fso, bool includeDirs = false) const;

		std::vector<FileSystemObject>::const_iterator begin();
		std::vector<FileSystemObject>::const_iterator end();
		u32 getFileObjectCount();
		u32 getFileCount();
		u32 getFolderCount();
		const FileSystemObject &operator[](u32 i);
		template<class T> bool isFile(T t);
		template<class T> bool isFolder(T t);
		template<class T> bool isRoot(T t);

	private:

		std::vector<FileSystemObject> files;
		u32 folderC, fileC;
	};

	template<> bool FileSystem::isFile(std::string str);
	template<> bool FileSystem::isFile(u32 i);

	template<> bool FileSystem::isFolder(std::string str);
	template<> bool FileSystem::isFolder(u32 i);

	template<> bool FileSystem::isRoot(std::string str);
	template<> bool FileSystem::isRoot(u32 i);

	template<class T>
	const T &FileSystem::getResource(std::string str) const {

		static_assert(std::is_base_of<GenericResourceBase, T>::value, "operator<T>[] where T should be instanceof GenericResourceBase");

		try {

			const FileSystemObject &fso = (*this)[str];

			if (!fso.isFile()) {
				throw(std::exception("Resource is a folder and couldn't be cast to GenericResourceBase"));
			}

			u32 resource = fso.resource;
			return get<T>(resource);
		}
		catch (std::exception e) {
			throw(std::exception(std::string("Couldn't get resource (\"").append(e.what()).append("\")").c_str()));
		}
	}

	template<class T>
	const T &FileSystem::getResource(const FileSystemObject &fso) const {

		static_assert(std::is_base_of<GenericResourceBase, T>::value, "operator<T>[] where T should be instanceof GenericResourceBase");

		try {

			if (!fso.isFile()) {
				throw(std::exception("Resource is a folder and couldn't be cast to GenericResourceBase"));
			}

			u32 resource = fso.resource;
			return get<T>(resource);
		}
		catch (std::exception e) {
			throw(std::exception(std::string("Couldn't get resource (\"").append(e.what()).append("\")").c_str()));
		}
	}

	struct FolderInfo {
		u32 offset;
		u16 firstFilePosition;
		u16 relation;
	};
}