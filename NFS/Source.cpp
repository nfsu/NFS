#ifndef __LIBDLL__

#include <stdio.h>
#include "NHelper.h"
using namespace nfs;

void test1(Buffer buf) {

	u32 NCLR_off = 0x5EBCE5C;
	u32 NCGR_off = 0x5EB0E1C;
	u32 NARC_off = 0x5EA2600;

	NARC narc = NARC();
	//auto m = getArchiveFpMap<NFactory, void*, Buffer>(ArchiveTypes());
	runArchiveFunction<NFactory>(MagicNumber::get<NARC>, ArchiveTypes(), (void*)&narc, offset(buf, NARC_off));

	//NType::readGenericResource(&narc, offset(buf, NARC_off));

	NArchieve arch;
	NType::convert(narc, &arch);

	NHelper::writeNCGR(buf, NCLR_off, NCGR_off, "Final.png");
}

void test2(Buffer buf) {

	u32 NCLR_off = 0x5E8E630;
	u32 NCGR_off = 0x5E8D5F0;

	NHelper::writeNCGR(buf, NCLR_off, NCGR_off, "Final0.png");
}

void test3(Buffer buf) {

	u32 NCLR_off = 0x5EBCE5C;
	u32 NCGR_off = 0x5EB0E1C;
	u32 NCSR_off = 0x5EB05F8;

	NHelper::writeNCSR(buf, NCLR_off, NCGR_off, NCSR_off, "Final1.png");
}


void test4(Buffer buf) {

	u32 NCLR_off = 0x5E88454;
	u32 NCGR_off = 0x5E86FC4;
	u32 NCSR_off = 0x5E8867C;

	NHelper::writeNCSR(buf, NCLR_off, NCGR_off, NCSR_off, "Final2.png");
}

void test5() {

	std::string path("ROM.nds"); //TODO: !!!

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