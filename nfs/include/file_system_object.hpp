#pragma once
#include "generic.hpp"

namespace nfs {

	struct FileSystemObject {

		String name = "";
		Buffer buf;

		Buffer compressed;
		usz reservedSize = 0;

		u32 *filePtrs{}, *filePtr1{};

		usz folders = 0, files = 0, objects = 0, index = 0;
		usz parent = 0, resource = 0, fileHint = 0, folderHint = 0;
		usz indexInFolder = 0;

		static constexpr usz root = usz_MAX, folder = usz_MAX - 1;

		inline bool isFolder() const { return resource >= folder; }
		inline bool isFile() const { return !isFolder(); }
		inline bool isRoot() const { return resource == root; }
		inline bool hasParent() const { return resource < root; }

		inline bool isCompressed() const { return compressed.size(); }

		inline bool contains(Buffer origin, usz addr) const {
			u8 *off = origin.add(addr);
			return off >= buf.begin() && off < buf.end();
		}

		//Compresses and stores the result into where it's really stored
		void commit();

		bool operator==(const FileSystemObject &other) { return index == other.index; }
	};
}