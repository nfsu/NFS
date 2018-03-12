#pragma once
#include "archive.h"
#include "filesystemobject.h"

namespace nfs {

	class FileSystem : public Archive {

	public:

		FileSystem();
		FileSystem(NDS *rom);
		FileSystem(const FileSystem &fs);
		FileSystem &operator=(const FileSystem &fs);
		FileSystem(FileSystem &&fs);

		std::vector<FileSystemObject> &getFileSystem();
		std::vector<FileSystemObject>::iterator begin();
		std::vector<FileSystemObject>::iterator end();

		FileSystemObject &operator[](u32 i);
		FileSystemObject *operator[](std::string path);
		ArchiveObject &getResource(FileSystemObject &fso);

		std::vector<FileSystemObject*> operator[](FileSystemObject &fso);
		std::vector<FileSystemObject*> traverse(FileSystemObject &fso, bool includeDirs = true, bool includeSubfiles = false);

		u32 size() const;
		u32 getFiles() const;
		u32 getFolders() const;

		template<typename ...args>
		FileSystemObject *foreachInFolder(bool(*isMatch)(FileSystem &fs, FileSystemObject &fso, u32 i, u32 index, args... arg), FileSystemObject &start, args... arg);

		template<typename T>
		T &get(std::string path);

		template<typename T>
		T &get(FileSystemObject &fso);

		template<typename T>
		T &get(ArchiveObject &ao);

		void clear();

	protected:

		void copy(const FileSystem &fs);

	private:

		std::vector<FileSystemObject> fileSystem;
		u32 files, folders;
	};


	template<typename ...args>
	FileSystemObject *FileSystem::foreachInFolder(bool(*isMatch)(FileSystem &fs, FileSystemObject &fso, u32 i, u32 index, args... arg), FileSystemObject &start, args... arg) {

		u32 folderC = 0;
		for (u32 i = start.folderHint; i < folders && folderC < start.folders; ++i) {

			FileSystemObject &fso = fileSystem[i];
			if (fso.parent == start.index) {

				if (isMatch(*this, fso, folderC, i, arg...))
					return &fso;

					++folderC;
			}
		}

		u32 fileC = 0;
		for (u32 i = start.fileHint; i < folders + files && fileC < start.files; ++i) {

			FileSystemObject &fso = fileSystem[i];
			if (fso.parent == start.index) {

				if (isMatch(*this, fso, fileC, i, arg...))
					return &fso;

				++fileC;
			}
		}

		return nullptr;
	}

	template<typename T>
	T &FileSystem::get(std::string path) {

		FileSystemObject *fso = (*this)[path];

		if (fso == nullptr)
			throw std::exception(("FileSystem Couldn't get fso at path \"" + path + "\"").c_str());

		return get<T>(*fso);
	}

	template<typename T>
	T &FileSystem::get(FileSystemObject &fso) {
		return get<T>(getResource(fso));
	}

	template<typename T>
	T &FileSystem::get(ArchiveObject &ao) {
		return Archive::get<T>(ao);
	}
}