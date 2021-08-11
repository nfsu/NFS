#include "compression_helper.hpp"
#include "resource_helper.hpp"
#include <algorithm>
using namespace nfs;

f32 CompressionHelper::normalize(u32 val, u32 maxVal) {
	return f32(val) / maxVal;
}

u8 CompressionHelper::fromBGR5(u8 val) {
	return u8(val / 31.f * 255);
}

u32 CompressionHelper::samplePixel(u16 pix) {

	u32 b = fromBGR5(pix & 0x1FU);
	u32 g = fromBGR5((pix & 0x3E0U) >> 5U);
	u32 r = fromBGR5((pix & 0x7C00U) >> 10U);

	return (0xFFU << 24U) | (r << 16U) | (g << 8U) | b;
}

f32x3 CompressionHelper::sampleColor(u16 pix) {

	f32 b = normalize(pix & 0x1FU, 0x1FU);
	f32 g = normalize((pix & 0x3E0U) >> 5U, 0x1FU);
	f32 r = normalize((pix & 0x7C00U) >> 10U, 0x1FU);

	return { r, g, b };
}

u32 CompressionHelper::unnormalize(f32 val, u32 maxVal) {
	return u32(val * maxVal);
}

u8 CompressionHelper::toBGR5(u8 val) {
	return u8(val / 255.f * 31);
}

u16 CompressionHelper::storePixel(u32 pix) {

	u32 r = toBGR5(pix & 0xFFU);
	u32 g = toBGR5((pix & 0xFF00U) >> 8U);
	u32 b = toBGR5((pix & 0xFF0000U) >> 16U);

	return b | (g << 5U) | (r << 10U);
}

u16 CompressionHelper::storeColor(const f32x3 &val) {

	u16 r = u16(unnormalize(val[0], 0x1FU));
	u16 g = u16(unnormalize(val[1], 0x1FU));
	u16 b = u16(unnormalize(val[2], 0x1FU));

	return b | (g << 5U) | (r << 10U);

}

u32 CompressionHelper::generateRandom(u32 seed, u32 multiply, u32 add) {
	return multiply * seed + add;
}

CompressionType CompressionHelper::getCompressionType(const u8 *tag, usz len) {

	auto type = ResourceHelper::getMagicNum(ResourceHelper::getType(tag, len));

	if (len < 5 || type != NBUO_num)
		return CompressionType::None;

	auto dat = CompressionType(*tag);

	if (*tag >> 4 == 0x2)					//Exception for Huff
		return CompressionType::Huffman;

	for (const CompressionType &type : compressionTypes)
		if (dat == type)
			return dat;

	return CompressionType::None;
}

Buffer CompressionHelper::compress(Buffer in, Buffer result, bool isDecompression) {
	
	//bool b = false;
	//
	//if(
	//	in.size() > 9 && 
	//	ResourceHelper::getMagicNum(ResourceHelper::getType(in.add(5), in.size() - 5)) !=
	//	NBUO_num
	//)
	//	b = true;

	//This is what I call "YOLO compression"
	//just try it; if it doesn't work, it's not compression

	try {

		switch (getCompressionType(in.add(), in.size())) {

			case CompressionType::Copy:
				return compressCopy(in, result, isDecompression);

			case CompressionType::LZ77:
				return compressLZ77(in, result, isDecompression);

			case CompressionType::LZ11:
				return compressLZ11(in, result, isDecompression);

			case CompressionType::Huffman:
				return compressHuffman(in, result, isDecompression);

			case CompressionType::RLE:
				return compressRLE(in, result, isDecompression);

			case CompressionType::LZ40:
				return compressLZ40(in, result, isDecompression);
		}

	} CATCH(e) { 
	
		//if (b)
		//	__debugbreak();
	}

	return in;
}

usz CompressionHelper::getDecompressionAllocation(Buffer in) {

	switch (getCompressionType(in.add(), in.size())) {

		case CompressionType::LZ77:
		case CompressionType::LZ11:
		case CompressionType::LZ40:

			//LZnn sometimes has the following u32 contain length. Probably if 24 bit is not enough (>16MB files)

			if (!(in.at<u32>() >> 8) && in.size() >= 8)
				return in.at<u32>(4);

		case CompressionType::Huffman:
		case CompressionType::RLE:
			return in.at<u32>() >> 8;

	}

	return 0;
}

Buffer CompressionHelper::compressCopy(Buffer in, Buffer res, bool isDecompression) {

	if (!isDecompression)
		EXCEPTION("Unimplemented compressCopy");		//TODO:

	u32 decompSize = in.at<u32>() >> 8;

	if (decompSize + 0x4 != in.size())
		EXCEPTION("Invalid decomp size");

	return in.offset(4);
}

Buffer CompressionHelper::compressLZ77(Buffer in, Buffer res, bool isDecompression) {
	return compressLZBase(in, res, isDecompression, CompressionType::LZ77);
}

Buffer CompressionHelper::compressLZ11(Buffer in, Buffer res, bool isDecompression) {
	return compressLZBase(in, res, isDecompression, CompressionType::LZ11);
}

//LZ keeps certain bytes inline until it finds the same pattern of characters later
//Then it references that by pointing back to it.
//For example: my boy, my boy
//Would turn into: my boy, (go back to start, til start + 6 bytes)

Buffer CompressionHelper::compressLZBase(Buffer in, Buffer res, bool isDecompression, CompressionType type) {

	if (isDecompression && res.size() && res.size() != getDecompressionAllocation(in))
		EXCEPTION("Mismatching pre-defined size");

	if (!isDecompression)
		EXCEPTION("Unimplemented compressLZBase");	//TODO:

	u32 decompSize = getDecompressionAllocation(in);
	in.addOffset((in.at<u32>() >> 8) == 0 ? 8 : 4);

	bool allocated = false;

	if (!res.size()) {
		res = Buffer::alloc(decompSize);
		allocated = true;
	}

	FINALLY(
		if (allocated)
			res.dealloc();
	);

	Buffer resPtr = res;

	//Repeated pattern

	u8 mask = 1, flags = 0;

	while (in.size() && resPtr.size()) {

		//This part basically does the whole 8-block for loop

		if (mask != 1) [[likely]]
			mask >>= 1;

		else {
			flags = in.consume();
			mask = 0x80;
		}

		if (!(flags & mask)) [[likely]]
			resPtr.append(in.consume());

		else if(type == CompressionType::LZ77) {

			//Displacement is apparently stored weirdly. You'd think byte count would be in low nibble, not high

			u8 compressionInfo = in.consume();

			u8 bytes = (compressionInfo >> 4) + 3;

			u16 disp = ((u16(compressionInfo & 0xF) << 8) | in.consume()) + 1;

			if (disp > resPtr.add() - res.add())
				EXCEPTION("Disp pointer was invalid. Pointed back too much");

			//Copy the N+3 bytes located -disp relative to our current output

			for (u8 i = 0; i < bytes; ++i)
				resPtr.append(resPtr.at(-usz(disp)));

		}

		//LZ11 or 11LZS or whatever

		else if (type == CompressionType::LZ11) {

			u8 b0 = in.consume(), b1, b2;
			u32 disp{}, bytes{};

			switch (b0 >> 4) {		//Indicator

				//273 letter 'words'

				case 0:
					b1 = in.consume();
					bytes = u16(u8(b0 << 4) | (b1 >> 4)) + 0x11;
					disp = (u16(b1 & 0xF) << 8) | in.consume();
					break;

				//65 809 letter 'words'

				case 1:
					b1 = in.consume();
					b2 = in.consume();
					bytes = u32((b2 >> 4) | (u16(b1) << 4) | (u16(b0 & 0xF) << 12)) + 0x111;
					disp = (u16(b2 & 0xF) << 8) | in.consume();
					break;

				//16 letter 'words'

				default:
					bytes = (b0 >> 4) + 1;
					disp = (u16(b0 & 0xF) << 8) | in.consume();
					break;
			}

			++disp;

			if (disp > resPtr.add() - res.add())
				EXCEPTION("Disp pointer was invalid. Pointed back too much");

			for (u32 i = 0; i < bytes; ++i)
				resPtr.append(resPtr.at(-usz(disp)));
		}
	}

	if (resPtr.size())
		EXCEPTION("Couldn't consume entire input and output buffer. Invalid compression");

	END_FINALLY;
	return res;
}

//Huffman counts characters in a string and then creates nodes based on count
//it groups more often used characters in more common nodes that use less bits

Buffer CompressionHelper::compressHuffman(Buffer in, Buffer res, bool isDecompression) {

	if (isDecompression && res.size() && res.size() != getDecompressionAllocation(in))
		EXCEPTION("Mismatching pre-defined size");

	if (!isDecompression)
		EXCEPTION("Compression not yet supported");

	u32 datSiz = in.at() & 0xF;

	if (datSiz != 4 && datSiz != 8)
		EXCEPTION("Invalid Huffman header");

	u32 decompSize = getDecompressionAllocation(in);
	in.addOffset(4);

	bool allocated = false;

	if (!res.size()) {
		res = Buffer::alloc(decompSize);
		allocated = true;
	}

	FINALLY(
		if (allocated)
			res.dealloc();
	);

	Buffer resPtr = res;

	//Build BSP 

	/*u16 sizeOfTreeTable = (u16(in.consume()) + 1) * 2;

	for (u16 i = 0; i < sizeOfTreeTable; ++i) {

		u8 treeNode = in.consume();
		u8 off = (treeNode & 0x1F) * 2 + 2;

		//Is A character

		if (treeNode & 0x40) {

		}

		//Is B character

		if (treeNode & 0x80) {

		}

	}

	...

	//
*/

	END_FINALLY;
	return res;
}

//Very simple; each block of about 128-130 bytes we have a flag if it's compressed
//and the size. If it's compressed we copy the next byte N times, otherwise we copy
//the next N bytes.

Buffer CompressionHelper::compressRLE(Buffer in, Buffer res, bool isDecompression) {

	if (isDecompression && res.size() && res.size() != getDecompressionAllocation(in))
		EXCEPTION("Mismatching pre-defined size");

	if (!isDecompression)
		EXCEPTION("Compression not yet supported");

	u32 decompSize = getDecompressionAllocation(in);
	in.addOffset(4);

	bool allocated = false;

	if (!res.size()) {
		res = Buffer::alloc(decompSize);
		allocated = true;
	}

	FINALLY(
		if (allocated)
			res.dealloc();
	);

	Buffer resPtr = res;

	while(in.size() && resPtr.size()) {

		u8 flag = in.consume();

		bool isCompressed = flag & 0x80;
		u8 rl = (flag & 0x7F) + (isCompressed ? 3 : 1);

		if (isCompressed) {
			in.requireSize(1);
			resPtr.requireSize(rl);
			std::memset(resPtr.add(), in.consume(), rl);
			resPtr.addOffset(rl);
		}

		else {
			in.requireSize(rl);
			resPtr.requireSize(rl);
			std::memcpy(resPtr.add(), in.add(), rl);
			resPtr.addOffset(rl);
			in.addOffset(rl);
		}
	}

	if (resPtr.size()) {
		res.dealloc();
		EXCEPTION("Invalid RLE file");
	}

	END_FINALLY;
	return res;
}

Buffer CompressionHelper::compressLZ40(Buffer in, Buffer res, bool isDecompression) {

	return res;
}
