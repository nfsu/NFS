#pragma once
#include "archive.hpp"
#include "file_system_object.hpp"

namespace nfs {

	class FileSystem : public Archive {

	public:

		FileSystem();
		FileSystem(NDS *rom);

		FileSystem(const FileSystem &fs);
		FileSystem &operator=(FileSystem &&fs);
		FileSystem &operator=(const FileSystem &fs);
		FileSystem(FileSystem &&fs);

		~FileSystem() { clear(); }

		inline ListIt<FileSystemObject> begin() { return fileSystem.begin(); }
		inline ListIt<FileSystemObject> end() { return fileSystem.end(); }

		inline const List<FileSystemObject> &getFileSystem() const { return fileSystem; }
		inline FileSystemObject &operator[](usz i) { return fileSystem[i]; }

		inline ListItConst<FileSystemObject> begin() const { return fileSystem.begin(); }
		inline ListItConst<FileSystemObject> end() const { return fileSystem.end(); }

		inline List<FileSystemObject> &getFileSystem() { return fileSystem; }
		inline const FileSystemObject &operator[](usz i) const { return fileSystem[i]; }

		FileSystemObject *operator[](const String &path);

		inline ArchiveObject &getResource(FileSystemObject &fso) {
			return Archive::operator[](fso.resource);
		}

		List<FileSystemObject*> operator[](FileSystemObject &fso);
		List<FileSystemObject*> traverse(FileSystemObject &fso, bool includeDirs = true, bool includeSubfiles = false);

		inline usz size() const { return fileSystem.size(); }
		inline usz getFiles() const { return files; }
		inline usz getFolders() const { return folders; }

		template<typename ...args>
		FileSystemObject *foreachInFolder(bool(*isMatch)(FileSystem &fs, FileSystemObject &fso, usz i, usz index, args... arg), FileSystemObject &start, args... arg);

		template<typename T>
		T &get(const String &path) {

			if (FileSystemObject *fso = (*this)[path])
				return get<T>(*fso);

			EXCEPTION(("FileSystem Couldn't get fso at path \"" + path + "\"").c_str());
		}

		template<typename T>
		T &get(FileSystemObject &fso) {
			return get<T>(getResource(fso));
		}

		template<typename T>
		T &get(ArchiveObject &ao) {
			return Archive::get<T>(ao);
		}

		inline void clear() {
			fileSystem.clear();
			Archive::clear();
			files = folders = supportedFiles = 0;
		}

		inline usz getSupportedFiles() const { return supportedFiles; }

	protected:

		void _copy(const FileSystem &other);
		void _move(FileSystem &&other);

	private:

		List<FileSystemObject> fileSystem;
		usz files{}, folders{}, supportedFiles{};
	};


	template<typename ...args>
	FileSystemObject *FileSystem::foreachInFolder(bool(*isMatch)(FileSystem &fs, FileSystemObject &fso, usz i, usz index, args... arg), FileSystemObject &start, args... arg) {

		usz folderC = 0;
		for (usz i = start.folderHint; i < folders && folderC < start.folders; ++i) {

			FileSystemObject &fso = fileSystem[i];
			if (fso.parent == start.index) {

				if (isMatch(*this, fso, folderC, i, arg...))
					return &fso;

				++folderC;
			}
		}

		usz fileC = 0;
		for (usz i = start.fileHint; i < folders + files && fileC < start.files; ++i) {

			FileSystemObject &fso = fileSystem[i];
			if (fso.parent == start.index) {

				if (isMatch(*this, fso, fileC, i, arg...))
					return &fso;

				++fileC;
			}
		}

		return nullptr;
	}
}