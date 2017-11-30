#ifndef __LIBDLL__

#include <stdio.h>
#include "NHelper.h"
using namespace nfs;

void test1(Buffer buf) {

	u32 NCLR_off = 0x5EBCE5C;
	u32 NCGR_off = 0x5EB0E1C;
	//u32 NARC_off = 0x5EA2600;		//Does work
	//u32 NARC_off = 0x5C9F000;		//Doesn't work?
	//u32 NARC_off = 0x5F32000;		//Does work
	u32 NARC_off = 0x5C78E00;		//Does work

	NARC narc = NARC();
	runArchiveFunction<NType::NFactory>(MagicNumber::get<NARC>, ArchiveTypes(), (void*)&narc, offset(buf, NARC_off));

	//NType::readGenericResource(&narc, offset(buf, NARC_off));

	NArchieve arch;
	NType::convert(narc, &arch);

	for (u32 i = 0; i < arch.size(); ++i) {

		printf("%u %s %u\n", i, arch.getTypeName(i).c_str(), arch.getType(i));

		try {
			NCLR &nclr = arch.operator[]<NCLR>(i);
			printf("Palette with dimension %ux%u\n", nclr.contents.front.dataSize / 2 / nclr.contents.front.c_colors, nclr.contents.front.c_colors);
		}
		catch (std::exception e) {

			try {
				NBUO &nbuo = arch.operator[]<NBUO>(i);
				printf("Couldn't parse %u; %s (%u)\n", i, nbuo.contents.front.name.c_str(), nbuo.contents.front.magicNumber);
			}
			catch (std::exception e) {

			}

		}
	}

	try {

		NDS nds = NType::readNDS(buf);
		FileSystem files;
		NType::convert(nds, &files);

		for (auto iter = files.begin(); iter != files.end(); ++iter) {
			FileSystemObject val = *iter;
			u32 resource = val.resource;
			if (val.isFile()) {
				printf("%s\n", val.name.c_str());
				///Get not working?
			} else {

			}
		}

	}
	catch (std::exception e) {
		printf("Error %s\n", e.what());
	}

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