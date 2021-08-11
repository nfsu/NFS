#pragma once
#include "generic.hpp"

namespace nfs {

	struct NFSP_Header {
		c8 magicNumber[4];
		u32 blocks, registers;
		u64 size;
	};

	struct NFSP_BlockHeader {
		u64 offset, length;
	};

	struct NFSP_Block : NFSP_BlockHeader {

		NFSP_Block(u64 offset, u64 length, Buffer buf):
			NFSP_BlockHeader { offset, length }, buf(buf) {}

		NFSP_Block(): NFSP_BlockHeader{} {}

		Buffer buf;

		bool operator<(const NFSP_Block &other) const {
			return length < other.length || (length == other.length && offset < other.offset);
		}
	};

	//Patcher class; stores modified bytes
	//Header:
	//NFSP (File System Patch; MagicNumber) (4 bytes; char[4])
	//#xx xx xx xx (Blocks of modified data) (4 bytes; u32)
	//#xx xx xx xx (Registers) (4 bytes; u32)
	//#xx xx xx xx xx xx xx xx (Modified file size) (8 bytes; u64)
	//Per block:
	//#xx xx xx xx (Offset since last offset) (4 bytes; u32)
	//#xx xx xx xx (Length in bytes (y)) (4 bytes; u32)
	//Buffer (Information to append) (y bytes)
	class Patcher {

	public:

		//Patches the path 'original' with the patch at path 'patch'
		//Outputs to 'out' path.
		//Returns bool success
		static bool patch(const String &original, const String &patch, const String &out);

		//Patches the Buffer 'original' with the Buffer 'patch'
		//Returns Buffer result (null buffer if invalid)
		static Buffer patch(Buffer original, Buffer patch);

		//Compares the files at 'original' and 'modified' and creates a patch at 'patch'
		//Returns bool success
		static bool writePatch(const String &original, const String &modified, const String &patch);

		//Compares the buffers 'original' and 'modified'
		//Returns Buffer patch (null buffer if invalid)
		static Buffer writePatch(Buffer original, Buffer modified);

	private:

		static bool compare(Buffer a, Buffer b, List<Buffer> &result, u32 &compares);

	};

}