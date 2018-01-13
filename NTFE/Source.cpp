#include "Link.h"
#include <QApplication.h>
#include <qicon.h>
#include "MainWindow.h"

#include <Patcher.h>

int main(int argc, char *argv[]) {

	nfs::Patcher::writePatch("ROM.nds", "ROMMod.nds", "RomDif.NFSP");

	printf("%u\n", (u32)sizeof(Buffer));

	QApplication app(argc, argv);
	QIcon icon = QIcon("Resources/NFS.png");

	MainWindow wind;
	wind.setWindowIcon(icon);
	wind.show();

	return app.exec();
}