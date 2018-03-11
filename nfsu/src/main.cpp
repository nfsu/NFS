#include <QtWidgets/qwidget.h>
#include <QtWidgets/qapplication.h>
#include <QtGui/qicon.h>

#include <settings.h>

class MainWindow : public QWidget {

public:

	MainWindow() {}
	~MainWindow() {}

};

int main(int argc, char *argv[]) {

	QApplication app(argc, argv);
	QIcon icon = QIcon("resources/logo.png");

	MainWindow wind;
	wind.setWindowIcon(icon);
	wind.show();

	return app.exec();
}