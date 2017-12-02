#ifndef __LIBDLL__

#include <stdio.h>
#include <algorithm>
#include "NHelper.h"
using namespace nfs;

void test1(Buffer buf) {

	try {

		NDS nds = NType::readNDS(buf);
		FileSystem files;
		NType::convert(nds, &files);

		std::vector<const FileSystemObject*> fsos = files[files["fielddata"]];
		for (u32 i = 0; i < fsos.size(); ++i)
			printf("%s\n", fsos[i]->path.c_str());


		for (auto iter = files.begin(); iter != files.end(); ++iter) {

			FileSystemObject val = *iter;
			if (val.isFile()) {

				std::string name;
				u32 magicNumber;
				bool valid = val.getMagicNumber(name, magicNumber);

				try {
					const NBUO &nbuo = files.getResource<NBUO>(val);
					printf("Object: %s (%s)\n", val.path.c_str(), name.c_str());
				}
				catch (std::exception e) {

					try {
						const NCLR &nclr = files.getResource<NCLR>(val);
						Texture2D palette;
						NType::convert(*const_cast<NCLR*>(&nclr), &palette);
						std::string outdir = val.path;
						outdir = outdir.substr(0, outdir.size() - 1 - val.getExtension().size()) + ".png";
						std::replace(outdir.begin(), outdir.end(), '/', '-');

						writeTexture(palette, outdir);
					}
					catch (std::exception e) {

					}

					printf("Supported object: %s (%s)\n", val.path.c_str(), name.c_str());
				}
			}
			else {
				printf("Directory: %s\n", val.path.c_str());
			}
		}
	}
	catch (std::exception e) {
		printf("%s\n", e.what());
	}
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