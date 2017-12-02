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
		const T &operator[](std::string str) const {

			static_assert(std::is_base_of<GenericResourceBase, T>::value, "operator<T>[] where T should be instanceof GenericResourceBase");

			try {

				FileSystemObject &fso = (*this)[str];

				u32 resource = fso.resource;

				if (!fso.isFile()) {
					throw(std::exception("Resource is a folder and couldn't be cast to GenericResourceBase"));
					return *nullptr;
				}

				T *t = NType::castResource<T>(resources[resource]);

				if (t == nullptr)
					throw(std::exception("Couldn't convert resource"));

				return *t;
			}
			catch (std::exception e) {
				throw(std::exception(std::string("Couldn't get resource (\"").append(e.what()).append("\")")));

				return *nullptr;
			}
		}

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
		const FileSystemObject &operator[](u32 i);
		template<class T> bool isFile(T t);
		template<class T> bool isFolder(T t);
		template<class T> bool isRoot(T t);

	private:

		std::vector<FileSystemObject> files;

	};

	template<> bool FileSystem::isFile(std::string str);
	template<> bool FileSystem::isFile(u32 i);

	template<> bool FileSystem::isFolder(std::string str);
	template<> bool FileSystem::isFolder(u32 i);

	template<> bool FileSystem::isRoot(std::string str);
	template<> bool FileSystem::isRoot(u32 i);

	struct FolderInfo {
		u32 offset;
		u16 firstFilePosition;
		u16 relation;
	};

	template<> bool NType::convert(NDS nds, FileSystem *fs) {
		
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
			} else {

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

		u32 totalFiles = fso.size() - folderArraySize;
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
			} catch (std::exception e) {

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
}