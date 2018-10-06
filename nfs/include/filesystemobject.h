#pragma once

#include "generic.h"

namespace nfs {

	struct FileSystemObject {

		std::string name = "";
		Buffer buf;

		u16 folders = 0, files = 0, objects = 0, index = 0;
		u16 parent = 0, resource = 0, fileHint = 0, folderHint = 0;
		u16 indexInFolder = 0;

		bool isFolder() const { return resource >= u16_MAX - 1; }
		bool isFile() const { return !isFolder(); }
		bool isRoot() const { return resource == u16_MAX; }
		bool hasParent() const { return resource < u16_MAX; }

		bool operator==(const FileSystemObject &other) { return index == other.index; }

	};

}