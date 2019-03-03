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
using namespace nfs::arm;
using namespace nfs::arm::thumb;

int main(int argc, char *argv[]) {

	#define TEST_ARMULATOR
	#ifdef TEST_ARMULATOR

	u16 myAsm[] = {
		Op::mov(r0, 33),
		Op::mov(r1, 69),
		Op::add(r0, 12),
		Op::sub(r0, 9),
		Op::cmp(r0, 36),
		Op::b(NE, 4),			//r0 != 36 :end
		Op::mov(r0, 3),
		Op::add(r0, r1),
		Op::sub(r0, r1),
		Op::lsl(r0, 2),
		Op::lsr(r0, 3),			//end:
		Op::b(1),				//:next
		Op::mov(r1, 5),
		Op::mov(r0, 5),			//next:
		Op::bl(1, true),		//goto :myFunc
		Op::bl(1, false),		//goto :myFunc
		Op::lsl(r0, 2),
		Op::mov(r0, 3)			//myFunc:
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