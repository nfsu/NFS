#pragma once

#include "generic.h"
#include <string>
#include <vector>

namespace nfs {

	struct NFSP_Header {
		char magicNumber[4];
		u32 blocks, size, registers;
	};

	struct NFSP_Block {
		u32 offset, length;
		Buffer buf;

		bool operator<(const NFSP_Block &other) const {
			return length < other.length || (length == other.length && offset < other.offset);
		}
	};

	struct NFSP_Register {
		u32 size, count;
	};

	//Patcher class; stores modified bytes
	//Header:
	//NFSP (File System Patch; MagicNumber) (4 bytes; char[4])
	//#xx xx xx xx (Blocks of modified data) (4 bytes; u32)
	//#xx xx xx xx (Modified file size) (4 bytes; u32)
	//#xx xx xx xx (Padding; flags?) (4 bytes; u32)
	//Per block:
	//#xx xx xx xx (Offset since last offset) (4 bytes; u32)
	//#xx xx xx xx (Length in bytes (y)) (4 bytes; u32)
	//Buffer (Information to append) (y bytes)
	class Patcher {

	public:

		//Patches the path 'original' with the patch at path 'patch'
		//Outputs to 'out' path.
		//Returns bool success
		static bool patch(std::string original, std::string patch, std::string out);

		//Patches the Buffer 'original' with the Buffer 'patch'
		//Returns Buffer result (null buffer if invalid)
		static Buffer patch(Buffer original, Buffer patch);

		//Compares the files at 'original' and 'modified' and creates a patch at 'patch'
		//Returns bool success
		static bool writePatch(std::string original, std::string modified, std::string patch);

		//Compares the buffers 'original' and 'modified'
		//Returns Buffer patch (null buffer if invalid)
		static Buffer writePatch(Buffer original, Buffer modified);

	private:

		static bool compare(Buffer a, Buffer b, std::vector<Buffer> &result, u32 &compares);

	};

}