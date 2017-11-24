//#include "NTypes.h"
#include <stdio.h>
//
//bool hasPath = true;

//void test1() {
//	String mainPath = newString3("D:/programming/hacking/Pocket monsters/roms/Pokemon Fabula.nds");
//	String path0 = newString3("D:/programming/hacking/Pocket monsters/textures/Intro Screen.png");
//	String path1 = newString3("D:/programming/hacking/Pocket monsters/textures/Intro Screen2.png");
//	String path2 = newString3("D:/programming/hacking/Pocket monsters/textures/Intro Screen3.png");
//	String path3 = newString3("D:/programming/hacking/Pocket monsters/textures/Dawn.png");
//	String path4 = newString3("D:/programming/hacking/Pocket monsters/textures/Lucas.png");
//	String path5 = newString3("D:/programming/hacking/Pocket monsters/textures/Intro Screen-full.png");
//	String path6 = newString3("D:/programming/hacking/Pocket monsters/textures/Intro Screen-platinum.png");
//	String path7 = newString3("D:/programming/hacking/Pocket monsters/textures/Intro Screen.bin");
//
//	Buffer buf = readFile(mainPath);
//
//	PT2D introScreen = readNPT2D(buf, 0x5EB03D0, 0x5EA2684);
//	PT2D introScreen2 = readNPT2D(buf, 0x610E6D0, 0x610E8F8);
//	PT2D introScreen3 = readNPT2D(buf, 0x5EBCE5C, 0x5EB0E1C);
//	PT2D dawn = readNPT2D(buf, 0x5E92958, 0x5E91918);
//	PT2D lucas = readNPT2D(buf, 0x5E8E630, 0x5E8D5F0);
//
//	Buffer result = newBuffer1(552 + 49216 + 2084);
//
//	PTT2D introScreen0 = readNPTT2D(buf, 0x5EBCE5C, 0x5EB0E1C, 0x5EB05F8);
//	PTT2D introScreen1 = readNPTT2D(buf, 0x5E88454, 0x5E86FC4, 0x5E8867C);
//
//	writePT2D(introScreen, path0, false);
//	writePT2D(introScreen2, path1, false);
//	writePT2D(introScreen3, path2, false);
//	writePT2D(dawn, path3, false);
//	writePT2D(lucas, path4, false);
//
//	writePTT2D(introScreen0, path5, false, false);
//	writePTT2D(introScreen1, path6, false, false);
//
//	writeNPTT2D(result, 0, 552, 552 + 49216, introScreen0);
//	writeBuffer(result, path7);
//
//	deletePT2D(&introScreen3);
//	deletePT2D(&introScreen2);
//	deletePT2D(&introScreen);
//	deletePT2D(&dawn);
//	deletePT2D(&lucas);
//
//	deletePTT2D(&introScreen0);
//	deletePTT2D(&introScreen1);
//
//	deleteBuffer(&result);
//	deleteString(&path7);
//
//	deleteBuffer(&buf);
//	deleteString(&mainPath);
//	deleteString(&path0);
//	deleteString(&path1);
//	deleteString(&path2);
//	deleteString(&path3);
//	deleteString(&path4);
//	deleteString(&path5);
//	deleteString(&path6);
//}
//
//void test2() {
//	String mainPath = newString3("D:/programming/hacking/Pocket monsters/roms/Pokemon Fabula.nds");
//	Buffer buf = readFile(mainPath);
//
//	NARC narc = createNARC(offset(buf, 0x5EA2600));
//	deleteNARC(&narc);
//
//	deleteBuffer(&buf);
//}

#include "NTypes2.h"
using namespace nfs;

void test3() {

	///If you want to use this test; get a rom and set NCLR_off, NCGR_off and NARC_off.
	///And set the path to the rom's path

	Buffer buf = readFile(path);

	NCLR nclr;
	NType::readGenericResource(&nclr, offset(buf, NCLR_off));

	NCGR ncgr;
	NType::readGenericResource(&ncgr, offset(buf, NCGR_off));

	NARC narc;
	NType::readGenericResource(&narc, offset(buf, NARC_off));

	NArchieve arch;
	NType::convert(narc, &arch);

	deleteBuffer(&buf);
}

int main() {
	test3();
	getchar();
	return 0;
}