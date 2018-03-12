#include "window.h"
#include "nexplorer.h"
#include <patcher.h>
#include <QtGui/qdesktopservices.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qurl.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qlayout.h>
using namespace nfsu;
using namespace nfs;

Window::Window() {

	setWindowTitle("File System Utilities");

	setupUI();
}

Window::~Window() {
	rom.dealloc();
}


///UI Actions

void Window::setupUI() {
	setupLayout();
	setupToolbar();
	setupExplorer(layout);
}

void Window::setupLayout() {
	setLayout(layout = new QVBoxLayout);
}

void Window::setupToolbar() {

	QMenuBar *qtb;
	layout->addWidget(qtb = new QMenuBar());

	QMenu *file = qtb->addMenu("File");
	QMenu *view = qtb->addMenu("View");
	QMenu *options = qtb->addMenu("Options");
	QMenu *help = qtb->addMenu("Help");

	///File
	QAction *load = file->addAction("Load");
	QAction *exp = file->addAction("Export");
	QAction *imp = file->addAction("Import");
	QAction *reload = file->addAction("Reload");
	QAction *save = file->addAction("Save");
	QAction *find = file->addAction("Find");

	connect(load, &QAction::triggered, this, [&]() { this->load(); });
	connect(exp, &QAction::triggered, this, [&]() { this->exportPatch(); });
	connect(imp, &QAction::triggered, this, [&]() { this->importPatch(); });
	connect(reload, &QAction::triggered, this, [&]() { this->reload(); });
	connect(save, &QAction::triggered, this, [&]() { this->write(); });
	//TODO: Implement 'find file'

	///View
	QAction *restore = view->addAction("Reset");

	connect(restore, &QAction::triggered, this, [&]() { this->restore(); });

	///Options
	QAction *preferences = options->addAction("Preferences");

	//TODO: Implement 'preferences'

	///Help
	QAction *documentation = help->addAction("Documentation");

	connect(documentation, &QAction::triggered, this, [&]() {this->documentation(); });

}

void Window::setupExplorer(QLayout *layout) {

	explorer = new NExplorer(fileSystem);

	NExplorerView *view = new NExplorerView(explorer);
	layout->addWidget(view);

	view->addResourceCallback(u32_MAX, [this](FileSystem &fs, FileSystemObject &fso, ArchiveObject &ao, const QPoint &point) {
		this->activateResource(fso, ao, point);
	});
}

void Window::activateResource(FileSystemObject &fso, ArchiveObject &ao, const QPoint &point) {

	QMenu contextMenu(tr(ao.name.c_str()), this);

	QAction *view = contextMenu.addAction("View");
	QAction *viewb = contextMenu.addAction("View data");
	QAction *expr = contextMenu.addAction("Export resource");
	QAction *impr = contextMenu.addAction("Import resource");
	QAction *info = contextMenu.addAction("Info");

	connect(view, &QAction::triggered, this, [&]() { this->viewResource(fso, ao); });
	connect(viewb, &QAction::triggered, this, [&]() { this->viewData(fso, ao); });
	connect(expr, &QAction::triggered, this, [&]() { this->exportResource(fso, ao); });
	connect(impr, &QAction::triggered, this, [&]() { this->importResource(fso, ao); });
	connect(info, &QAction::triggered, this, [&]() { this->info(fso, ao); });

	contextMenu.exec(mapToGlobal(point));
}

///File actions

void Window::load() {

	if (rom.ptr != nullptr) {

		QMessageBox::StandardButton reply = QMessageBox::question(this, "Load ROM", "Loading a ROM will clear all resources and not save any progress. Do you want to continue?");

		if (reply == QMessageBox::No)
			return;
	}

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

	fileSystem.clear();
	rom.dealloc();

	rom = Buffer::read(file.toStdString());

	if (rom.ptr != nullptr) {
		NDS *nds = (NDS*) rom.ptr;
		setWindowTitle(QString("File System Utilities: ") + nds->title);
		fileSystem = nds;
	}

	restore();
}

void Window::write() {

	QString file = QFileDialog::getSaveFileName(this, tr("Save ROM"), "", tr("NDS file (*.nds)"));

	if (file == "" || !file.endsWith(".nds", Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid .nds file to write to");
		messageBox.setFixedSize(500, 200);
		return;
	}

	write(file);
}

void Window::write(QString file) {
	if (rom.ptr != nullptr)
		rom.write(file.toStdString());
}

void Window::exportPatch() {
	QString file = QFileDialog::getSaveFileName(this, tr("Export Patch"), "", tr("NFS Patch file (*.NFSP)"));
	exportPatch(file);
}

void Window::exportPatch(QString file) {

	if (rom.ptr == nullptr) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a ROM before exporting a patch");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer original = Buffer::read(this->file.toStdString());

	if (original.ptr == nullptr) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Couldn't load the original ROM. Please reload the ROM");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer patch = Patcher::writePatch(original, rom);
	original.dealloc();
	
	if (patch.ptr == nullptr) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Couldn't complete patch; files were probably identical");
		messageBox.setFixedSize(500, 200);
		return;
	}

	patch.write(file.toStdString());
	patch.dealloc();
}

void Window::importPatch() {

	QMessageBox::StandardButton reply = QMessageBox::question(this, "Import Patch", "Importing a patch might damage the ROM, or might not work if applied on the wrong ROM. Do you want to continue?");

	if (reply == QMessageBox::No)
		return;

	QString file = QFileDialog::getOpenFileName(this, tr("Apply Patch"), "", tr("NFS Patch file (*.NFSP)"));

	if (file == "" || !file.endsWith(".NFSP", Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid .NFSP file through File->Apply patch");
		messageBox.setFixedSize(500, 200);
		return;
	}

	importPatch(file);
}

void Window::importPatch(QString file) {

	Buffer buf = Buffer::read(file.toStdString());

	if (buf.ptr == nullptr || rom.ptr == nullptr) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "To apply a patch, please select a valid file and ROM");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer patched = Patcher::patch(rom, buf);
	buf.dealloc();
	rom.dealloc();
	rom = patched;
}

///View

void clearLayout(QLayout *layout) {

	while (QLayoutItem *item = layout->takeAt(0)) {

		if (QWidget *widget = item->widget())
			widget->deleteLater();

		if (QLayout *childLayout = item->layout())
			clearLayout(childLayout);

		delete item;
	}

	delete layout;
}

void Window::restore() {
	clearLayout(layout);
	setupUI();
}

///Help

void Window::documentation() {
	QDesktopServices::openUrl(QUrl("https://github.com/Nielsbishere/NFS/tree/NFS_Reloaded"));
}

///Right click resource actions

void Window::viewResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {
	//TODO: Select all editors that can display the resource
	//TODO: Make user pick if > 0
}

void Window::viewData(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {
	//TODO: Select all editors that can display binary
	//TODO: Make user pick if > 0
}

void Window::exportResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {

	QString name = QString(ao.name.c_str()).split("/").last();
	QString extension = name.split(".").last();

	QString file = QFileDialog::getSaveFileName(this, tr("Save Resource"), "", tr((extension + " file (*." + extension + ")").toStdString().c_str()));

	if (file == "" || !file.endsWith("." + extension, Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid ." + extension + " file to write to");
		messageBox.setFixedSize(500, 200);
		return;
	}

	if (!fso.buf.write(file.toStdString())) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Couldn't write resource");
		messageBox.setFixedSize(500, 200);
		return;
	}
}

void Window::importResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {

	QString name = QString(ao.name.c_str()).split("/").last();
	QString extension = name.split(".").last();

	QString file = QFileDialog::getOpenFileName(this, tr("Load Resource"), "", tr((extension + " file (*." + extension + ")").toStdString().c_str()));

	if (file == "" || !file.endsWith("." + extension, Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid ." + extension + " file to read from");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer buf = Buffer::read(file.toStdString());

	if (buf.size != fso.buf.size) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Resources can't change size");
		messageBox.setFixedSize(500, 200);
		buf.dealloc();
		return;
	}

	memcpy(fso.buf.ptr, buf.ptr, buf.size);
	buf.dealloc();
}

void Window::info(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {
	if(ao.info.magicNumber != ResourceHelper::getMagicNumber<NBUO>())
		QDesktopServices::openUrl(QUrl("https://github.com/Nielsbishere/NFS/tree/NFS_Reloaded/docs/resource" + QString(ao.info.type) + ".md"));
	else
		QDesktopServices::openUrl(QUrl("https://github.com/Nielsbishere/NFS/tree/NFS_Reloaded/docs/README.md"));
}