#include "window.hpp"
#include "nexplorer.hpp"
#include "info_window.hpp"
#include "patcher.hpp"
#include <QtGui/qdesktopservices.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qurl.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qapplication.h>
#include <QtGui/qevent.h>
#include "model.hpp"
#include "palette_editor.hpp"
#include "tile_editor.hpp"
#include "game_editor.hpp"

using namespace nfsu;
using namespace nfs;

//TODO: Fix memory leak

Window::Window() {

	setWindowTitle("File System Utilities");
	setMinimumSize(QSize(1300, 750));

	setStyleSheet(

		"QWidget {"
			"background: #303030;"
			"background-color: #303030;"
			"color: SkyBlue;"
			"border: 0px;"
			"selection-background-color: #404040;"
			"alternate-background-color: #303030;"
			"font: 14px;"
		"}"
		
		"QPushButton {"
			"border: 1px solid #101010;"
			"background: #202020;"
		"}"

		"QMenu {"
			"border: 1px solid #202020;"
			"background: #404040;"
		"}"

		"QTreeView::item:hover, QAbstractItemView::item:hover, QMenuBar::item:hover, QMenu::item:hover, QTabBar::tab:hover {"
			"color: DeepSkyBlue;"
		"}"

		"QTreeView::item:selected, QAbstractItemView::item:selected, QMenuBar::item:selected, QMenu::item:selected, QTabBar::tab:selected {"
			"color: DeepSkyBlue;"
			"background: #505050;"
		"}"

		"QTabWidget::pane {"
			"border: 2px solid #101010;"
		"}"

		"QTabBar::tab, QTabBar::tab:selected {"
			"background: #202020;"
			"margin-right: 5px;"
			"padding-top: 2px;"
			"padding-bottom: 2px;"
			"padding-left: 2px;"
		"}"

		"QTabBar::tab:selected {"
			"background: #505050;"
			"color: DeepSkyBlue;"
		"}"

		"QHeaderView::section {"
			"background-color: #303030;"
			"border: 0px #101010;"
		"}"

		"QTreeView, QTableView, QMenuBar {"
			"border: 1px solid #101010;"
		"}"

		"QScrollBar {"
			"border: 2px solid #404040;"
			"background: #303030;"
			"color: SkyBlue;"
		"}"

		"QScrollBar::add-page, QScrollBar::sub-page {"
			"background: none;"
		"}"

	);

	setupUI();
}

Window::~Window() {
	rom.dealloc();
}

//UI Actions

void Window::setupUI() {
	setupLayout();
	setupToolbar();
	setupInfoWindow();
	setupExplorer();
	setupTabs(rightLayout);
}

void Window::setupLayout() {

	setCentralWidget(central = new QWidget());

	central->setLayout(layout = new QHBoxLayout());
	splitter = new QSplitter();
	layout->addWidget(splitter);

	splitter->addWidget(left = new QSplitter(Qt::Vertical));

	splitter->addWidget(right = new QWidget());
	right->setLayout(rightLayout = new QVBoxLayout());

	QList<int> size = QList<int>{ 0 /* minimum */, 1 /* everything */ };
	splitter->setSizes(size);
}

int StopAlt::styleHint(StyleHint stylehint, const QStyleOption *opt, const QWidget *widget, QStyleHintReturn *returnData) const {

	if (stylehint == QStyle::SH_MenuBar_AltKeyNavigation)
		return 0;

	return QProxyStyle::styleHint(stylehint, opt, widget, returnData);
}

void Window::setupToolbar() {

	QMenuBar *qtb = new QMenuBar(0);

	setMenuBar(qtb);

	QMenu *file = qtb->addMenu("File");
	QMenu *view = qtb->addMenu("View");
	QMenu *options = qtb->addMenu("Options");
	QMenu *help = qtb->addMenu("Help");

	//File

	QAction *load = file->addAction("Load");
	QAction *save = file->addAction("Save As");
	QAction *saveCurrent = file->addAction("Save");
	QAction *reload = file->addAction("Reload");

	file->addSeparator();

	QAction *exp = file->addAction("Export");
	QAction *imp = file->addAction("Import");

	file->addSeparator();

	QAction *find = file->addAction("Find");
	QAction *filter = file->addAction("Filter");
	QAction *order = file->addAction("Order");

	connect(load, &QAction::triggered, this, &Window::_load);
	connect(exp, &QAction::triggered, this, &Window::_exportPatch);
	connect(imp, &QAction::triggered, this, &Window::_importPatch);
	connect(reload, &QAction::triggered, this, &Window::_reload);
	connect(save, &QAction::triggered, this, &Window::_write);
	connect(saveCurrent, &QAction::triggered, this, [&]() { this->write(this->file); });
	connect(find, &QAction::triggered, this, &Window::findFile);
	connect(filter, &QAction::triggered, this, &Window::filterFiles);
	connect(order, &QAction::triggered, this, &Window::orderFiles);

	//View

	QAction *restore = view->addAction("Reset");
	QAction *customize = view->addAction("Customize");

	connect(restore, &QAction::triggered, this, &Window::restore);
	connect(customize, &QAction::triggered, this, &Window::customize);

	//Options

	QAction *preferences = options->addAction("Preferences");

	connect(preferences, &QAction::triggered, this, &Window::preferences);

	//Help

	QAction *about = help->addAction("About");
	QAction *shortcuts = help->addAction("Shortcuts");
	QAction *documentation = help->addAction("Documentation");

	connect(about, &QAction::triggered, this, &Window::about);
	connect(shortcuts, &QAction::triggered, this, &Window::shortcuts);
	connect(documentation, &QAction::triggered, this, &Window::documentation);
}

void Window::setupExplorer() {

	explorer = new NExplorer(fileSystem);

	auto *view = explorerView = new NExplorerView(explorer);
	view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	left->addWidget(view);

	view->addResourceCallback(true, u32_MAX, [this, view](FileSystem &fs, FileSystemObject &fso, ArchiveObject &ao, const QPoint &point) {
		activateResource(fso, ao, view->mapToGlobal(point));
	});

	view->addExplorerCallback(false, [this](FileSystem &fs, FileSystemObject &fso, const QPoint &point) {

		if (fso.isFile())
			inspect(fso, fs.getResource(fso));

		else inspectFolder(fso);
	});
}

void Window::setupInfoWindow() {

	left->addWidget(infoWindow = new InfoWindow(true, true, this));

	infoWindow->setMinimumWidth(400);
	infoWindow->setMinimumHeight(210);

	infoWindow->setString("Path", "");
	infoWindow->setString("Folders", "");
	infoWindow->setString("Files", "");
	infoWindow->setString("Id", "");
	infoWindow->setString("Type", "");
	infoWindow->setString("Offset", "");
	infoWindow->setString("Length", "");
}

void Window::setupTabs(QLayout *layout) {

	editors.resize(6);

	for (auto &elem : editors)
		elem = nullptr;

	GameEditor *gameEditor = new GameEditor;
	editors[0] = gameEditor;

	PaletteEditor *paletteEditor = new PaletteEditor;
	editors[1] = paletteEditor;

	TileEditor *tileEditor = new TileEditor;
	editors[2] = tileEditor;

	tabs = new QTabWidget;
	tabs->addTab(gameEditor, QIcon("resources/folder.png"), "Game editor");			//TODO: Edit game
	tabs->addTab(paletteEditor, QIcon("resources/palette.png"), "Palette editor");
	tabs->addTab(tileEditor, QIcon("resources/tilemap.png"), "Tile editor");
	tabs->addTab(new QWidget, QIcon("resources/map.png"), "Tilemap editor");			//TODO: Edit tilemap
	tabs->addTab(new QWidget, QIcon("resources/model.png"), "Model editor");			//TODO: Edit model
	tabs->addTab(new QWidget, QIcon("resources/binary.png"), "File editor");			//TODO: Edit binary or text

	//TODO: Map file spaces (find available space, etc)

	tabs->setCurrentIndex(selectedId);
	tabs->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	selected = editors[tabs->currentIndex()];

	if (selected != nullptr) {
		selected->showInfo(infoWindow);
		selected->onSwap();
	}

	connect(tabs, &QTabWidget::currentChanged, this, [&](int idx) { 

		if(selected != nullptr)
			selected->hideInfo(infoWindow);

		selected = editors[idx];
		selectedId = idx; 

		if (selected != nullptr) {
			selected->showInfo(infoWindow);
			selected->onSwap();
		}

	});

	rightLayout->addWidget(tabs);
}

void Window::activateResource(FileSystemObject &fso, ArchiveObject &ao, const QPoint &point) {

	QMenu contextMenu(tr(ao.name.c_str()), this);

	QAction *view = contextMenu.addAction("View resource");
	QAction *viewData = contextMenu.addAction("View data");

	contextMenu.addSeparator();

	QAction *expr = contextMenu.addAction("Export resource");
	QAction *impr = contextMenu.addAction("Import resource");

	contextMenu.addSeparator();

	QAction *info = contextMenu.addAction("Documentation");

	connect(view, &QAction::triggered, this, [&]() { viewResource(fso, ao); });
	connect(viewData, &QAction::triggered, this, [&]() { this->viewData(ao.buf); });
	connect(expr, &QAction::triggered, this, [&]() { exportResource(fso, ao); });
	connect(impr, &QAction::triggered, this, [&]() { importResource(fso, ao); });
	connect(info, &QAction::triggered, this, [&]() { this->info(fso, ao); });

	contextMenu.exec(point);
}

//File actions

void Window::_load() {

	if (rom.add()) {

		QMessageBox::StandardButton reply = QMessageBox::question(nullptr, "Load ROM", "Loading a ROM will clear all resources and discard any progress. Do you want to continue?");

		if (reply == QMessageBox::No)
			return;
	}

	QString file = QFileDialog::getOpenFileName(nullptr, tr("Open ROM"), "", tr("NDS file (*.nds)"));
	
	if (file == "" || !file.endsWith(".nds", Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid .nds file through File->Load");
		messageBox.setFixedSize(500, 200);
		return;
	}

	load(file);
}

void Window::load(const QString &_file) {
	file = _file;
	reload();
}

void Window::reload() {

	fileSystem.clear();
	rom.dealloc();

	rom = Buffer::readFile(file.toStdString());

	if (NDS *nds = rom.add<NDS>()) {
		setWindowTitle(QString("File System Utilities: ") + nds->title);
		fileSystem = nds;
	}

	restore();
}

void Window::_reload() {

	if (rom.add()) {

		QMessageBox::StandardButton reply = QMessageBox::question(nullptr, "Reload ROM", "Reloading a ROM will clear all resources and discard any progress. Do you want to continue?");

		if (reply == QMessageBox::No)
			return;
	}

	reload();
}

void Window::_write() {

	QString file = QFileDialog::getSaveFileName(this, tr("Save ROM"), "", tr("NDS file (*.nds)"));

	if (file == "" || !file.endsWith(".nds", Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid .nds file to write to");
		messageBox.setFixedSize(500, 200);
		return;
	}

	write(file);
}

void Window::write(const QString &file) {
	if (rom.add())
		rom.writeFile(file.toStdString());
}

void Window::_exportPatch() {
	QString file = QFileDialog::getSaveFileName(this, tr("Export Patch"), "", tr("NFS Patch file (*.NFSP)"));
	exportPatch(file);
}

void Window::exportPatch(const QString &file) {

	if (!rom.add()) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a ROM before exporting a patch");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer original = Buffer::readFile(file.toStdString());

	if (!original.add()) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Couldn't load the original ROM. Please reload the ROM");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer patch = Patcher::writePatch(original, rom);
	original.dealloc();
	
	if (!patch.add()) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Couldn't complete patch; files were probably identical");
		messageBox.setFixedSize(500, 200);
		return;
	}

	patch.writeFile(file.toStdString());
	patch.dealloc();
}

void Window::_importPatch() {

	//TODO: Verify checksum to see if our files are the same (patch can have multiple checksums where one of them has to be applied)

	QMessageBox::StandardButton reply = QMessageBox::question(nullptr, "Import Patch", "Importing a patch might damage the ROM, or might not work if applied on the wrong ROM. Do you want to continue?");

	if (reply == QMessageBox::No)
		return;

	QString file = QFileDialog::getOpenFileName(nullptr, tr("Apply Patch"), "", tr("NFS Patch file (*.NFSP)"));

	if (file == "" || !file.endsWith(".NFSP", Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid .NFSP file through File->Apply patch");
		messageBox.setFixedSize(500, 200);
		return;
	}

	importPatch(file);
}

void Window::importPatch(const QString &file) {

	Buffer buf = Buffer::readFile(file.toStdString());

	if (!buf.add() || !rom.add()) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "To apply a patch, please select a valid file and ROM");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer patched = Patcher::patch(rom, buf);
	buf.dealloc();
	rom.dealloc();
	rom = patched;

	reload();
}

void Window::findFile() {
	//TODO: Example; find files with extension, name, directory, in folder, that are supported, etc.
}

void Window::filterFiles() {
	//TODO: Example; filter on extension, supported
}

void Window::orderFiles() {
	//TODO: Order on size, alphabetical, offset; ascending, descending
}

//View

void Window::restore() {

	NDS *nds = rom.add<NDS>();

	if (nds != nullptr) {
		infoWindow->setString("Path", "/");
		infoWindow->setString("Folders", QString::number(fileSystem.getFolders()));
		infoWindow->setString("Files", QString::number(fileSystem.getFiles()));
		infoWindow->setString("Supported files", QString::number(fileSystem.getSupportedFiles()));
	} else {
		infoWindow->setString("Folders", "");
		infoWindow->setString("Files", "");
	}

	infoWindow->setString("Id", "");
	infoWindow->setString("Type", "");
	infoWindow->setString("Offset", "");
	infoWindow->setString("Length", "");

	explorerView->reset();

	if (selected)
		selected->hideInfo(infoWindow);

	for (ResourceEditor *editor : editors)
		if (editor) {

			editor->reset();

			if (nds)
				editor->init(nds, fileSystem);
		}

	if (selected)
		selected->showInfo(infoWindow);
}

void Window::customize() {
	//TODO: Customize style sheet
}

void Window::preferences() {
	//TODO: Allow changing preferences
	//TODO: Save last folder & file & tab as preference
}

//Help

void Window::documentation() {
	QDesktopServices::openUrl(QUrl("https://github.com/NFSU/NFS/tree/NFS_Reloaded"));
}

void Window::shortcuts() {
	QDesktopServices::openUrl(QUrl("https://github.com/NFSU/NFS/tree/NFS_Reloaded/guide/shortcut.md"));
}

void Window::about() {
	QDesktopServices::openUrl(QUrl("https://github.com/NFSU/NFS/tree/NFS_Reloaded/nfsu"));
}

///Right click resource actions

void Window::viewResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {

	inspect(fso, ao);

	if (!selected || !selected->allowsResource(fso, ao)) {

		u32 i = 0;

		for(ResourceEditor *editor : editors)

			if (editor != nullptr && editor->isPrimaryEditor(fso, ao)) {
				tabs->setCurrentIndex(i);
				break;
			}
			
			else ++i;

		if(i == editors.size())
			return;
	}

	selected->inspectResource(fileSystem, fso, ao);
	selected->showInfo(infoWindow);
}

void Window::viewData(Buffer buf) {

	if (!selected || !selected->allowsData()) {

		u32 i = 0;

		for (ResourceEditor *editor : editors)

			if (editor != nullptr && editor->allowsData()) {
				tabs->setCurrentIndex(i);
				break;
			}

			else ++i;

		if (i == editors.size())
			return;

	}

	selected->inspectData(buf);
}

void Window::exportResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {

	QString name = QString(ao.name.c_str()).split("/").last();
	QString extension = name.split(".").last();
	QString fileType = extension + " file";

	if (extension.contains('?')) {
		extension = '*';
		fileType = "Any file";
	}

	QString file = QFileDialog::getSaveFileName(this, tr("Save Resource"), "", tr((fileType + " (*." + extension + ")").toStdString().c_str()));

	if (file == "" || (!file.endsWith("." + extension, Qt::CaseInsensitive) && extension != "*")) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid ." + extension + " file to write to");
		messageBox.setFixedSize(500, 200);
		return;
	}

	if (!fso.buf.writeFile(file.toStdString())) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Couldn't write resource");
		messageBox.setFixedSize(500, 200);
		return;
	}
}

void Window::importResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {

	QString name = QString(ao.name.c_str()).split("/").last();
	QString extension = name.split(".").last();
	QString fileType = extension + " file";

	if (extension.contains('?')) {
		extension = '*';
		fileType = "Any file";
	}

	QString file = QFileDialog::getOpenFileName(nullptr, tr("Load Resource"), "", tr((fileType + " (*." + extension + ")").toStdString().c_str()));

	if (file == "" || (!file.endsWith("." + extension, Qt::CaseInsensitive) && extension != "*")) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid ." + extension + " file to read from");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer buf = Buffer::readFile(file.toStdString());

	if (buf.size() != fso.buf.size()) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Resources can't change size");
		messageBox.setFixedSize(500, 200);
		buf.dealloc();
		return;
	}

	std::memcpy(fso.buf.add(), buf.add(), buf.size());
	buf.dealloc();
}

void Window::info(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {

	if (ao.info.magicNumber != ResourceHelper::getMagicNumber<NBUO>())
		QDesktopServices::openUrl(QUrl("https://github.com/NFSU/NFS/tree/NFS_Reloaded/docs/resource" + QString::number(ao.info.type) + ".md"));

	else QDesktopServices::openUrl(QUrl("https://github.com/NFSU/NFS/tree/NFS_Reloaded/docs"));
}

void Window::inspect(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {

	QString fileName = QString::fromStdString(fso.name);

	infoWindow->setString("Folders", QString::number(fso.folders));
	infoWindow->setString("Files", QString::number(fso.files));
	infoWindow->clearString("Supported files");

	infoWindow->setString("Path", fileName);
	infoWindow->setString("Id", QString::number(fso.index));

	if (ao.info.magicNumber != NBUO_num)
		infoWindow->setString("Type", QString::fromStdString(
			ResourceHelper::getName(nullptr, 0, ao.info.magicNumber, false)
		));

	else if (fileName.contains('.'))
		infoWindow->setString("Type", fileName.remove(0, fileName.lastIndexOf('.') + 1));

	else infoWindow->setString("Type", "Undefined");

	infoWindow->setString("Offset", QString("0x") + QString::number(u32(fso.buf.add() - rom.add()), 16));
	infoWindow->setString("Length", QString::number(fso.buf.size()));
}

void Window::inspectFolder(nfs::FileSystemObject &fso) {

	QString fileName = QString::fromStdString(fso.name);

	infoWindow->setString("Folders", QString::number(fso.folders));
	infoWindow->setString("Files", QString::number(fso.files));

	infoWindow->clearString("Supported files");

	infoWindow->setString("Path", fileName);
	infoWindow->setString("Id", QString::number(fso.index));

	infoWindow->setString("Type", "Folder");

	infoWindow->setString("Offset", "");
	infoWindow->setString("Length", "");
}