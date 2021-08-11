#pragma once
#include "generic.hpp"

namespace nfs {

	enum class CompressionType : u8 {
		Copy,							//Used in case it contains pure binary to avoid unintentional compression markers
		LZ77	= 0x10,
		LZ11	= 0x11,
		Huffman = 0x20,
		RLE		= 0x30,
		LZ40	= 0x40,
		None	= 0xFF
	};

	static constexpr CompressionType compressionTypes[] = {
		CompressionType::Copy,
		CompressionType::LZ77,
		CompressionType::LZ11,
		CompressionType::Huffman,
		CompressionType::RLE,
		CompressionType::LZ40
	};

	struct CompressionHelper {

		//Get a 16-bit color to linear color space
		static f32x3 sampleColor(u16 val);

		//Get a 16-bit color as a 32-bit color (full opacity; alpha 0xFF)
		static u32 samplePixel(u16 val);

		//Get a 32-bit color as a 16-bit color
		static u16 storePixel(u32 val);

		//Get a linear color as 16-bit
		static u16 storeColor(const f32x3 &val);

		//Get a pseudo random number
		//Also used in image "encryption"
		static u32 generateRandom(u32 seed, u32 multiply = 0x41c64e6d, u32 add = 0x6073);

		//Normalize an int in range [0, maxVal]
		static f32 normalize(u32 val, u32 maxVal);

		//Get a 5-bit channel as 8-bit
		static u8 fromBGR5(u8 val);

		//Unnormalize float range [0,1] to [0,maxVal]
		static u32 unnormalize(f32 val, u32 maxVal);

		//Get a 8-bit channel as 5-bit
		static u8 toBGR5(u8 val);

		//Used to determine if the file's data is compressed (check with CompressionType::NONE to make sure)
		//A len to avoid reading null size files or files that can't use compression
		static CompressionType getCompressionType(const u8 *tag, usz len);

		//Whether or not decompression needs to allocate some separate space and how much
		static usz getDecompressionAllocation(Buffer b);

		//Compress a file. You can use a pre-allocated version, but since size is not known, it might throw.
		static Buffer compress(Buffer b, Buffer result = {}) { return compress(b, result, false); }

		//Decompress a file; you can pre-allocate (by using getDecompressionAllocation) and then pass result
		//Otherwise it will return a newly allocated buffer
		static Buffer decompress(Buffer b, Buffer result = {}) { return compress(b, result, true); }

	private:

		static Buffer compress(Buffer in, Buffer result, bool isDecompression);

		static Buffer compressCopy(Buffer in, Buffer result, bool isDecompression);
		static Buffer compressLZ77(Buffer in, Buffer result, bool isDecompression);
		static Buffer compressLZ11(Buffer in, Buffer result, bool isDecompression);
		static Buffer compressHuffman(Buffer in, Buffer result, bool isDecompression);
		static Buffer compressRLE(Buffer in, Buffer result, bool isDecompression);
		static Buffer compressLZ40(Buffer in, Buffer result, bool isDecompression);

		static Buffer compressLZBase(Buffer in, Buffer result, bool isDecompression, CompressionType magic);
	};

}