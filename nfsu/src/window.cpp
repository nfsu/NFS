#include "window.h"
#include <QtGui/qdesktopservices.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qurl.h>
using namespace nfsu;

Window::Window() {
	load();
}

Window::~Window() {}

void Window::load() {

	QString file = QFileDialog::getOpenFileName(this, tr("Open ROM"), "", tr("NDS file (*.nds)"));
	
	if (file == "" || !file.endsWith(".nds", Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid .nds file through File->Load");
		messageBox.setFixedSize(500, 200);
		return;
	}

	load(file);
}

void Window::load(QString _file) {
	file = _file;
	reload();
}

void Window::reload() {

	rom.dealloc();
	fileSystem.clear();

	rom = Buffer::read(file.toStdString());

	if (rom.ptr != nullptr) {
		nfs::NDS *nds = (nfs::NDS*) rom.ptr;
		setWindowTitle(QString("File System Utilities: ") + nds->title);
		fileSystem = nds;
	}
}

void Window::documentation() {
	QDesktopServices::openUrl(QUrl("https://github.com/Nielsbishere/NFS"));
}

void Window::write(QString file) {
	if (rom.ptr != nullptr)
		rom.write(file.toStdString());
}

void Window::exportPatch(QString file) {

}

void Window::importPatch(QString file) {

}