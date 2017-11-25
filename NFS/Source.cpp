#ifndef __LIBDLL__

#include <stdio.h>
#include "NTypes2.h"
using namespace nfs;

void test1(Buffer buf) {

	u32 NCLR_off = 0;	//TODO: !!! Insert offset to palette here
	u32 NCGR_off = 0;	//TODO: !!! Insert offset to image here
	u32 NARC_off = 0;	//TODO: !!! Insert offset to archieve here

	NCLR nclr;
	NType::readGenericResource(&nclr, offset(buf, NCLR_off));

	NCGR ncgr;
	NType::readGenericResource(&ncgr, offset(buf, NCGR_off));

	NARC narc;
	NType::readGenericResource(&narc, offset(buf, NARC_off));

	NArchieve arch;
	NType::convert(narc, &arch);

	Texture2D palette;
	NType::convert(nclr, &palette);
	writeTexture(palette, "Palette_example.png");

	Texture2D tex;
	NType::convert(ncgr, &tex);

	Texture2D tex2 = convertToRGBA8({ tex.width, tex.height, palette, tex });
	writeTexture(tex2, "Final.png");
	deleteTexture(&tex2);
}

void test2(Buffer buf) {

	u32 NCLR_off = 0;	//TODO: !!! Insert offset to palette here
	u32 NCGR_off = 0;	//TODO: !!! Insert offset to image here

	NCLR nclr;
	NType::readGenericResource(&nclr, offset(buf, NCLR_off));

	NCGR ncgr;
	NType::readGenericResource(&ncgr, offset(buf, NCGR_off));

	Texture2D palette;
	NType::convert(nclr, &palette);
	writeTexture(palette, "Palette_example0.png");

	Texture2D tex;
	NType::convert(ncgr, &tex);

	Texture2D tex2 = convertToRGBA8({ tex.width, tex.height, palette, tex });
	writeTexture(tex2, "Final0.png");
	deleteTexture(&tex2);
}

void test3() {

	std::string path("!!! INSERT YOUR ROM NAME HERE !!!"); //TODO: !!!

	Buffer buf = readFile(path);

	test2(buf);
	test1(buf);

	deleteBuffer(&buf);
}

int main() {
	test3();
	getchar();
	return 0;
}

#endif