#include "window.h"

#include <QtWidgets/qapplication.h>
#include <QtGui/qicon.h>

#define USE_CONSOLE

#ifndef USE_CONSOLE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

using namespace nfsu;

int main(int argc, char *argv[]) {

	QApplication app(argc, argv);
	QIcon icon = QIcon("resources/logo.png");

	Window wind;
	wind.setWindowIcon(icon);
	wind.show();

	StopAlt stopAlt;

	app.setStyle(&stopAlt);

	return app.exec();
}