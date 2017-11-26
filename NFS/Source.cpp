#ifndef __LIBDLL__

#include <stdio.h>
#include "NHelper.h"
using namespace nfs;

void test1(Buffer buf) {

	u32 NCLR_off = 0x5EBCE5C;	//TODO: !!! Insert offset to palette here
	u32 NCGR_off = 0x5EB0E1C;	//TODO: !!! Insert offset to image here
	u32 NARC_off = 0x5EA2600;	//TODO: !!! Insert offset to archieve here

	NARC narc;
	NType::readGenericResource(&narc, offset(buf, NARC_off));

	NArchieve arch;
	NType::convert(narc, &arch);

	NHelper::writeNCGR(buf, NCLR_off, NCGR_off, "Final.png");
}

void test2(Buffer buf) {

	u32 NCLR_off = 0x5E8E630;	//TODO: !!! Insert offset to palette here
	u32 NCGR_off = 0x5E8D5F0;	//TODO: !!! Insert offset to image here

	NHelper::writeNCGR(buf, NCLR_off, NCGR_off, "Final0.png");
}

void test3(Buffer buf) {

	u32 NCLR_off = 0x5EBCE5C;	//TODO: !!! Insert offset to palette here
	u32 NCGR_off = 0x5EB0E1C;	//TODO: !!! Insert offset to image here
	u32 NCSR_off = 0x5EB05F8;	//TODO: !!! Insert offset to image here

	NHelper::writeNCSR(buf, NCLR_off, NCGR_off, NCSR_off, "Final1.png");
}


void test4(Buffer buf) {

	u32 NCLR_off = 0x5E88454;	//TODO: !!! Insert offset to palette here
	u32 NCGR_off = 0x5E86FC4;	//TODO: !!! Insert offset to image here
	u32 NCSR_off = 0x5E8867C;	//TODO: !!! Insert offset to image here

	NHelper::writeNCSR(buf, NCLR_off, NCGR_off, NCSR_off, "Final2.png");
}

void test5() {

	std::string path("D:/programming/hacking/Pocket monsters/roms/Pokemon Fabula.nds"); //TODO: !!!

	Buffer buf = readFile(path);

	test4(buf);
	test3(buf);
	test2(buf);
	test1(buf);

	deleteBuffer(&buf);
}

int main() {
	test5();
	getchar();
	return 0;
}

#endif