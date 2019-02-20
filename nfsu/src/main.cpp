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

	// a = 33
	// b = 69
	// a += 12		(a = 45)
	// a -= 9		(a = 36)
	// a cmp 36		(zero flag)
	// a = 3		(a = 3)
	// a += b		(72)
	// a -= b		(3)
	// a <<= 2		(12)
	// a >>= 3		(1)

	//0b00100 /* = */ 000 /* a */ 00100001 /* 33 */,
	//0b00100 /* = */ 001 /* b */ 01000101 /* 69 */,
	//0b00110 /* += */ 000 /* a */ 00001100 /* 12 */,
	//0b00111 /* -= */ 000 /* a */ 00001001 /* 9 */,
	//0b00101 /* cmp */ 000 /* a */ 00100100 /* 36 */,
	//0b00100 /* = */ 000 /* a */ 00000011 /* 3 */,
	//0b00011 /* -=/+= */ 0 /* reg */ 0 /* + */ 001 /* b */ 000 /* a */ 000 /* a */,
	//0b00011 /* -=/+= */ 0 /* reg */ 1 /* - */ 001 /* b */ 000 /* a */ 000 /* a */,
	//0b00001 /* <<= */ 00010 /* 2 */ 000 /* a */ 000 /* a */,
	//0b00000 /* >>= */ 00011 /* 3 */ 000 /* a */ 000 /* a */

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