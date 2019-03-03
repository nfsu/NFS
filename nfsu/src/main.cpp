#include "window.h"

#include "armulator.h"

#include <QtWidgets/qapplication.h>
#include <QtGui/qicon.h>

#define USE_CONSOLE

#ifndef USE_CONSOLE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

using namespace nfs;
using namespace nfsu;

int main(int argc, char *argv[]) {

	#define TEST_ARMULATOR
	#ifdef TEST_ARMULATOR

	u16 myAsm[] = {
		0b0010000000100001,		//r0 = 33
		0b0010000101000101,		//r1 = 69
		0b0011000000001100,		//r0 += 12
		0b0011100000001001,		//r0 -= 9
		0b0010100000100100,		//r0 == 36
		0b1101000000000100,		//if true; jump to (end)
		0b0010000000000011,		//r0 = 3
		0b0001100001000000,		//r0 += r1
		0b0001101001000000,		//r0 -= r1
		0b0000000010000000,		//r0 <<= 2
		0b0000100011000000,		//(end) r0 >>= 3
		0b1110000000000001,		//goto (next)
		0b0010000100000101,		//r1 = 5
		0b0010000000000101,		//(next) r0 = 5
		0b1111000000000000,		//goto (myFunc) (LBH)
		0b1111100000000001,		//goto (myFunc) (LBL)
		0b0000000010000000,		//r0 <<= 2
		0b0010000000000011		//(myFunc) r0 = 3
	};

	Buffer buf = Buffer((u32) sizeof(myAsm), (u8*) myAsm);

	nfs::arm::Armulator test(buf, 0);
	test.getCPSR().thumbMode = 1;
	test.exec();

	#endif

	QApplication app(argc, argv);
	QIcon icon = QIcon("resources/logo.png");

	Window wind;
	wind.setWindowIcon(icon);
	wind.show();

	StopAlt stopAlt;

	app.setStyle(&stopAlt);

	return app.exec();
}