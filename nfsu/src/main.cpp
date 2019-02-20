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

	#ifdef TEST_ARMULATOR

	u16 myAsm[] = {
		0b0010000000100001,
		0b0010000101000101,
		0b0011000000001100,
		0b0011100000001001,
		0b0010100000100100,
		0b0010000000000011,
		0b0001100001000000,
		0b0001101001000000,
		0b0000000010000000,
		0b0000100011000000
	};

	Buffer buf = Buffer((u32) sizeof(myAsm), (u8*) myAsm);

	Armulator test(buf, 0);
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