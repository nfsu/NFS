#ifndef __LIBDLL__

#include <stdio.h>
#include <algorithm>
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

void test5() {

	std::string path("ROM.nds"); //TODO: !!!

	Buffer buf = readFile(path);

	test1(buf);

	deleteBuffer(&buf);
}

int main() {

	test5();
	getchar();
	return 0;
}

#endif