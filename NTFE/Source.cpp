#include "Link.h"
#include <QApplication.h>
#include <qicon.h>
#include "MainWindow.h"

#include <Patcher.h>

int main(int argc, char *argv[]) {

	nfs::Patcher::writePatch("ROM.nds", "ROMMod.nds", "RomDif.NFSP");
	nfs::Patcher::patch("ROM.nds", "RomDif.NFSP", "ROMMod2.nds");
	
	QApplication app(argc, argv);
	QIcon icon = QIcon("Resources/NFS.png");

	MainWindow wind;
	wind.setWindowIcon(icon);
	wind.show();

	return app.exec();
}