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
		ArchiveObject &getResource(FileSystemObject &fso);

	protected:

		void copy(const FileSystem &fs);

	private:

		std::vector<FileSystemObject> fileSystem;
	};

}