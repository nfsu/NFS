#include "Link.h"
#include <QApplication.h>
#include <qicon.h>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QIcon icon = QIcon("Resources/NFS.png");

	MainWindow wind;
	wind.setWindowIcon(icon);
	wind.show();

	return app.exec();
}