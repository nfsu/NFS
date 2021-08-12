#include "tile_editor.hpp"
#include "tile_renderer.hpp"
#include "info_window.hpp"
#include "palette_renderer.hpp"

#pragma warning(push, 0)
	#include <QtGui/qevent.h>
	#include <QtWidgets/qpushbutton.h>
	#include <QtWidgets/qmessagebox.h>
	#include <QtWidgets/qfiledialog.h>
	#include <QtWidgets/qgridlayout.h>
#pragma warning(pop)

using namespace nfsu;
using namespace nfs;

TileEditor::TileEditor() {

	//Renderer

	renderer = new TileRenderer();
	renderer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	//Buttons

	QWidget *paletteWidget = new QWidget();

	QGridLayout *gridLayout = new QGridLayout();

	QPushButton *exportButton = new QPushButton(QIcon(QPixmap("resources/export.png")), "");
	QPushButton *exportConvertButton = new QPushButton(QIcon(QPixmap("resources/export_converted.png")), "");
	QPushButton *importButton = new QPushButton(QIcon(QPixmap("resources/import.png")), "");
	QPushButton *importConvertButton = new QPushButton(QIcon(QPixmap("resources/import_converted.png")), "");
	QPushButton *paletteButton = new QPushButton(QIcon(QPixmap("resources/palette.png")), "");
	QPushButton *gridButton = new QPushButton(QIcon(QPixmap("resources/grid.png")), "");

	connect(paletteButton, &QPushButton::clicked, this, [&]() { setUsePalette(!getUsePalette()); });
	//connect(gridButton, &QPushButton::clicked, this, )

	connect(exportButton, &QPushButton::clicked, this, [&]() {

		QString filters;

		bool hasTiledData = renderer->getTexture().getWidth();
		bool hasPaletteData = renderer->getPalette().getWidth();

		if (tile)
			filters += "NCGR file (*.NCGR)";
		else if (hasTiledData)
			filters += "Tiled data (*.bin)";

		if (hasTiledData && hasPaletteData)
			filters += ";;";

		if (paletteWidget)
			filters += "NCLR file (*.NCLR)";
		else if (hasPaletteData)
			filters += "Palette data (*.bin)";

		if (!hasTiledData && !hasPaletteData) {
			QMessageBox messageBox;
			messageBox.critical(0, "Error", "Please select a palette and/or tiled image");
			messageBox.setFixedSize(500, 200);
			return;
		}

		QString filter;
		QString file = QFileDialog::getSaveFileName(nullptr, tr("Save resource"), "", filters, &filter);

		if (file == "")
			return;

		QFile ofile = file;
		ofile.open(QIODevice::WriteOnly);

		if (!ofile.isOpen()) {
			QMessageBox messageBox;
			messageBox.critical(0, "Error", "Please select a valid output");
			messageBox.setFixedSize(500, 200);
			return;
		}

		Buffer buf;

		if (filter == "NCGR file (*.NCGR)")
			buf = tile->toBuffer();
		else if (filter == "NCLR file (*.NCLR)")
			buf = this->palette->toBuffer();
		else if (filter == "Palette data (*.bin)")
			buf = renderer->getPalette().toBuffer();
		else buf = renderer->getTexture().toBuffer();

		ofile.write((const c8*) buf.add(), buf.size());
		ofile.close();
	});

	connect(exportConvertButton, &QPushButton::clicked, this, [&]() {

		QString filter;
		QString file = QFileDialog::getSaveFileName(nullptr, tr("Save converted resource"), "", "Converted file (*.png);;Palette file (*.png);;Tiled file (*.png)", &filter);

		if (file.isEmpty())
			return;

		bool hasTiledData = renderer->getTexture().getWidth();
		bool hasPaletteData = renderer->getPalette().getWidth();

		if ((!hasTiledData && !hasPaletteData) || !renderer->getUsePalette()) {
			QMessageBox messageBox;
			messageBox.critical(0, "Error", "Please select a palette and tiled image and enable the palette");
			messageBox.setFixedSize(500, 200);
			return;
		}

		Texture2D tex2d;

		if (filter == "Converted file (*.png)")
			tex2d = Texture2D(renderer->getTexture(), renderer->getPalette());
		else if (filter == "Palette file (*.png)")
			tex2d = renderer->getPalette().toRGBA8();

		else tex2d = renderer->getTexture().toRGBA8();

		tex2d.writeFile(file.toStdString());
	});

	//TODO: Import button, import convert

	gridLayout->addWidget(importButton, 0, 0);
	gridLayout->addWidget(importConvertButton, 1, 0);
	gridLayout->addWidget(exportButton, 0, 1);
	gridLayout->addWidget(exportConvertButton, 1, 1);
	gridLayout->addWidget(gridButton, 0, 2);
	gridLayout->addWidget(paletteButton, 1, 2);

	gridLayout->setMargin(5);
	paletteWidget->setLayout(gridLayout);
	paletteWidget->setContentsMargins(0, 0, 0, 0);
	paletteWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	//Layout

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(0);
	layout->setMargin(0);
	setLayout(layout);

	layout->addWidget(paletteWidget);
	layout->addWidget(renderer);

	//TODO: Separation between file explorer and editor data

	//TODO: Allow changing size, tool, color
}

void TileEditor::setPalette(Texture2D tex) {
	renderer->setPalette(tex);
}

Texture2D TileEditor::getPalette() {
	return renderer->getPalette();
}

void TileEditor::setTiles(Texture2D tex) {
	renderer->setTexture(tex);
}

void TileEditor::setUsePalette(bool b) {
	renderer->setUsePalette(b);
}

bool TileEditor::getUsePalette() {
	return renderer->getUsePalette();
}

Texture2D TileEditor::getTiles() {
	return renderer->getTexture();
}

bool TileEditor::allowsResource(FileSystemObject&, ArchiveObject &ao) {
	return ao.info.magicNumber == NCGR::getMagicNumber() || ao.info.magicNumber == NCLR::getMagicNumber();
}

void TileEditor::inspectResource(FileSystem &fileSystem, FileSystemObject &fso, ArchiveObject &ao) {

	if (ao.info.magicNumber == NCGR::getMagicNumber()) {

		NCGR &ncgr = fileSystem.get<NCGR>(ao);
		Texture2D tex = ncgr;

		setTiles(tex);

		tile = &ncgr;
		tileName = QString::fromStdString(fso.name);

	} else {

		NCLR &nclr = fileSystem.get<NCLR>(ao);

		setPalette(Texture2D(nclr));

		palette = &nclr;
		paletteName = QString::fromStdString(fso.name);
	}
}

bool TileEditor::isPrimaryEditor(FileSystemObject&, ArchiveObject &ao) {
	return ao.info.magicNumber == NCGR::getMagicNumber();
}

void TileEditor::onSwap() {
	renderer->updateTexture();
}

void TileEditor::reset() {
	renderer->reset();
	palette = nullptr;
	tile = nullptr;
	paletteName = "";
	tileName = "";
	repaint();
}

enum Property : u32 {

	TILE_NAME,
	WIDTH,
	HEIGHT,
	TILING,
	BIT_DEPTH,
	ENCRYPTION,
	UNDEFINED,
	PALETTE_NAME,
	COLORS,
	PALETTES,

	END,
	START = TILE_NAME
};

static const c8 *properties[] = {
	"Tile name",
	"Width",
	"Height",
	"Tiling",
	"Bit depth",
	"Encryption",
	"Undefined",
	"Palette name",
	"Colors",
	"Palettes",
};

inline void set(InfoWindow *info, Property prop, const QString &str) {
	info->setString(properties[prop], str);
}

void TileEditor::showInfo(InfoWindow *info) {

	set(info, TILE_NAME, tileName);

	if (tile != nullptr && renderer->getTexture().getWidth() != 0) {

		set(info, WIDTH, QString::number(renderer->getTexture().getWidth()));
		set(info, HEIGHT, QString::number(renderer->getTexture().getHeight()));
		set(info, TILING, QString::number(renderer->getTexture().getTiles()));
		set(info, BIT_DEPTH, QString::number(renderer->getTexture().getBitsPerPixel()));
		set(info, ENCRYPTION, tile->at<0>().isEncrypted ? "On" : "Off");

		set(info, UNDEFINED,
			QString::number(tile->at<0>().sizeHint0) + " " + QString::number(tile->at<0>().sizeHint1) + " " +
			QString::number(tile->at<0>().sizeHint2) + " " + QString::number(tile->at<0>().specialTiling)
		);

	} 
	else for (u32 i = START; i != PALETTE_NAME; ++i) 
		set(info, Property(i), "");

	set(info, PALETTE_NAME, paletteName);

	if (palette != nullptr && renderer->getPalette().getWidth() != 0) {

		set(info, COLORS, QString::number(renderer->getPalette().getWidth()));
		set(info, PALETTES, QString::number(renderer->getPalette().getHeight()));
	}

	else for (u32 i = PALETTE_NAME; i != END; ++i)
		set(info, Property(i), "");
}

void TileEditor::hideInfo(InfoWindow *info) {

	for (u32 i = START; i != END; ++i)
		info->clearString(properties[i]);
}