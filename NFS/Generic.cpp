#include "Types.h"
#include <stdlib.h>
#include <stdio.h>

void deleteBuffer(Buffer *b) {
	if (b->data != NULL) {
		free(b->data);
		b->data = NULL;
		b->size = 0;
	}
}

Buffer newBuffer1(u32 size) {
	Buffer b;
	b.data = (u8*)malloc(size);
	memset(b.data, 0, size);
	b.size = size;
	return b;
}

Buffer newBuffer2(u8 *ptr, u32 size) {
	Buffer b;
	b.data = ptr;
	b.size = size;
	return b;
}

bool copyBuffer(Buffer dest, Buffer src, u32 size, u32 offset) {
	if (dest.size < offset + size) return false;
	memcpy(dest.data + offset, src.data, size);
	return true;
}

Buffer offset(Buffer b, u32 off) {
	Buffer res;
	res.size = 0;
	res.data = NULL;
	if (b.size < off) return res;
	res.size = b.size - off;
	res.data = b.data + off;
	return res;
}

bool setUInt(Buffer b, u32 offset, u32 value) {
	if (b.size < offset + 4) return false;

	u32 *ptr = (u32*)(b.data + offset);
	ptr[0] = value;
	return true;
}

bool setUShort(Buffer b, u32 offset, u16 value) {
	if (b.size < offset + 2) return false;

	u16 *ptr = (u16*)(b.data + offset);
	ptr[0] = value;
	return true;
}

u16 getUShort(Buffer b, u32 offset) {
	if (b.size < offset + 2) return 0;
	u16 *ptr = (u16*)(b.data + offset);
	return ptr[0];
}

u32 getUInt(Buffer b) {
	if (b.size < 4) return 0;
	u32 *ptr = (u32*)b.data;
	return ptr[0];
}

Buffer readFile(std::string str) {
	Buffer nullbuf;
	nullbuf.data = NULL;
	nullbuf.size = 0;

	FILE *f = fopen(str.c_str(), "rb");
	if (f == NULL) {
		printf("Couldn't open file (%s)\n", str.c_str());
		return nullbuf;
	}

	fseek(f, 0, SEEK_END);
	u32 fsize = (u32)ftell(f);
	fseek(f, 0, SEEK_SET);

	Buffer res = newBuffer1(fsize);
	fread(res.data, res.size, 1, f);
	fclose(f);

	return res;
}

char hexChar(u8 i) {
	if (i < 10) return '0' + i;
	return 'A' + (i - 10);
}

std::string toHex(u8 i) {
	std::string str = std::string(' ', 2);
	str[0] = hexChar((i & 0xF0) >> 4);
	str[1] = hexChar(i & 0xF);
	return str;
}

std::string toHex16(u16 i) {
	std::string str = std::string(' ', 4);
	str[0] = hexChar((i & 0xF000) >> 12);
	str[1] = hexChar((i & 0xF00) >> 8);
	str[2] = hexChar((i & 0xF0) >> 4);
	str[3] = hexChar(i & 0xF);
	return str;
}

std::string toHex32(u32 i) {
	std::string str(' ', 8);
	str[0] = hexChar((i & 0xF0000000) >> 28);
	str[1] = hexChar((i & 0xF000000) >> 24);
	str[2] = hexChar((i & 0xF00000) >> 20);
	str[3] = hexChar((i & 0xF0000) >> 16);
	str[4] = hexChar((i & 0xF000) >> 12);
	str[5] = hexChar((i & 0xF00) >> 8);
	str[6] = hexChar((i & 0xF0) >> 4);
	str[7] = hexChar(i & 0xF);
	return str;
}

void printBuffer(Buffer b) {
	printf("Buffer with size %u and contents:\n", b.size);
	for (u32 i = 0; i < b.size; ++i) {
		std::string str = toHex(b.data[i]);
		printf("%s ", str.c_str());
		if((i + 1) % 32 == 0)
			printf("\n");
	}
	printf("\n");
}

void clearBuffer(Buffer b) {
	if (b.data == NULL) return;
	memset(b.data, 0, b.size);
}

bool writeBuffer(Buffer b, std::string path) {
	FILE *f = fopen(path.c_str(), "wb");
	if (f == NULL) {
		printf("Couldn't open file for writing (%s)\n", path.c_str());
		return false;
	}

	fwrite(b.data, b.size, 1, f);
	fclose(f);
	printf("Successfully wrote %u bytes to file (%s)\n", b.size, path.c_str());

	return true;
}

//Texture2D convertToT2D2(PTT2D t) {
//	Texture2D nulltex;
//	memset(&nulltex, 0, sizeof(nulltex));
//
//	Texture2D res = newTexture2D1(t.md.w * 8, t.md.h * 8);
//
//	u32 tw = t.td.w / 8;
//
//	for (u32 j = 0; j < res.h; ++j){
//		u32 prev = 0;
//		for (u32 i = 0; i < res.w; ++i) {
//
//			u32 k = j * res.w + i;
//
//			u32 mx = i / 8;								//X index in map
//			u32 my = j / 8;								//Y index in map
//
//			u32 tx = i % 8;								//X index in tile
//			u32 ty = j % 8;								//Y index in tile
//
//			u32 pm = my * t.md.w + mx;					//Position in map
//
//			u32 ptd = t.md.t.data[pm];					//Data of map tile
//			u32 ptt = (ptd & 0xFF0000) >> 16;			//Translate
//			u32 ptp = (ptd & 0xFF000000) >> 24;			//Palette
//			u32 ptm = ptd & 0xFFFF;						//Position in tilemap
//
//			u32 ptmx = ptm % tw;						//X tile position in tilemap
//			u32 ptmy = ptm / tw;						//Y tile position in tilemap
//
//			if (ptt & 0b10)
//				ty = 7 - ty;
//
//			if (ptt & 0b1)
//				tx = 7 - tx;
//
//			u32 tmx = ptmx * 8 + tx;					//X position in tilemap
//			u32 tmy = ptmy * 8 + ty;					//Y position in tilemap
//
//			if (tmx > t.td.w || tmy > t.td.h) {
//				printf("Couldn't convert PTT2D to T2D; invalid reference to tilemap in map\n");
//				return nulltex;
//			}
//
//			u32 tm = tmy * t.td.w + tmx;				//Position in tilemap
//			u32 tms = t.td.t.data[tm | (ptp << 4)];		//Tilemap sample
//
//			if (tms > t.pd.w * t.pd.h) {
//				printf("Couldn't convert PTT2D to T2D; invalid reference to palette in tilemap\n");
//				return nulltex;
//			}
//
//			res.t.data[j * res.w + i] = t.pd.t.data[tms];
//		}
//	}
//
//	return res;
//}