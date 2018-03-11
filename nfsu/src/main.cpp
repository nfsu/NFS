#include <QtWidgets/qwidget.h>
#include <QtWidgets/qapplication.h>
#include <QtGui/qicon.h>

#define USE_CONSOLE

#ifndef USE_CONSOLE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include <filesystem.h>
#include <texture.h>

class MainWindow : public QWidget {

public:

	MainWindow() {}
	~MainWindow() {}

};

int main(int argc, char *argv[]) {

	Buffer buf = Buffer::read("D:/programming/Github/NFS/ROM.nds");

	nfs::FileSystem fs((nfs::NDS*) buf.ptr);

	nfs::NCGR &ncgr = fs.get<nfs::NCGR>("data/ground0.NCGR");
	nfs::NCLR &nclr = fs.get<nfs::NCLR>("data/ground0.NCLR");
	nfs::NSCR &nscr = fs.get<nfs::NSCR>("data/ground0.NSCR");
	nfs::Texture2D tex = nfs::Texture2D(nscr, ncgr, nclr);
	tex.write("test.png");
	tex.dealloc();

	QApplication app(argc, argv);
	QIcon icon = QIcon("resources/logo.png");

	MainWindow wind;
	wind.setWindowIcon(icon);
	wind.show();

	return app.exec();
}