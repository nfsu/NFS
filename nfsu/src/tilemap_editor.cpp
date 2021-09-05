#include "tilemap_editor.hpp"
#include "tilemap_renderer.hpp"
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

TilemapEditor::TilemapEditor() {

	//Renderer

	renderer = new TilemapRenderer();
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
	connect(gridButton, &QPushButton::clicked, this, [&]() { setUseGrid(!getUseGrid()); });

	//connect(gridButton, &QPushButton::clicked, this, )

	connect(exportButton, &QPushButton::clicked, this, [&]() {

		QString filters;

		bool hasTilemapData = renderer->getTilemap().getWidth();
		bool hasTileData = renderer->getTiles().getWidth();
		bool hasPaletteData = renderer->getPalette().getWidth();

		static const c8 *ncgrFile = "NCGR file (*.NCGR)";
		static const c8 *nclrFile = "NCLR file (*.NCLR)";
		static const c8 *nscrFile = "NSCR file (*.NSCR)";
		static const c8 *tiledDataFile = "Tiled data (*.bin)";
		static const c8 *paletteDataFile = "Palette data (*.bin)";

		if (tile)
			filters += ncgrFile;
		else if (hasTileData)
			filters += tiledDataFile;

		if (hasTileData && hasPaletteData)
			filters += ";;";

		if (paletteWidget)
			filters += nclrFile;
		else if (hasPaletteData)
			filters += "";

		if(filters.size() && hasTilemapData)
			filters += ";;";

		if(hasTilemapData)
			filters += nscrFile;

		if (!hasTileData && !hasPaletteData && !hasTilemapData) {
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

		if (filter == ncgrFile)
			buf = tile->toBuffer();

		else if (filter == nclrFile)
			buf = palette->toBuffer();

		else if (filter == nscrFile)
			buf = tilemap->toBuffer();

		else if (filter == paletteDataFile)
			buf = renderer->getPalette().toBuffer();

		else buf = renderer->getTiles().toBuffer();

		ofile.write((const c8*) buf.add(), buf.size());
		ofile.close();
	});

	connect(exportConvertButton, &QPushButton::clicked, this, [&]() {

		static const c8 *convertedFile = "Converted file (*.png)";
		static const c8 *paletteFile = "Palette file (*.png)";
		static const c8 *tilesFile = "Tiles file (*.png)";
		static const c8 *tilemapFile = "Tilemap file (*.png)";

		QString filter;
		QString file = QFileDialog::getSaveFileName(nullptr, tr("Save converted resource"), "", QString(convertedFile) + ";;" + paletteFile + ";;" + tilesFile + ";;" + tilemapFile, &filter);

		if (file.isEmpty())
			return;

		bool hasTilemapData = renderer->getTilemap().getWidth();
		bool hasTileData = renderer->getTiles().getWidth();
		bool hasPaletteData = renderer->getPalette().getWidth();

		if ((!hasTileData && !hasPaletteData && !hasTilemapData) || !renderer->getUsePalette()) {
			QMessageBox messageBox;
			messageBox.critical(0, "Error", "Please select a palette, tilemap and tiled image and enable the palette");
			messageBox.setFixedSize(500, 200);
			return;
		}

		Texture2D tex2d;

		if (filter == convertedFile)
			tex2d = Texture2D(renderer->getTilemap(), renderer->getTiles(), renderer->getPalette());

		else if (filter == paletteFile)
			tex2d = renderer->getPalette().toRGBA8();

		else if (filter == tilemapFile)
			tex2d = renderer->getTilemap().toRGBA8(false);	//TODO: Fix this

		else tex2d = renderer->getTiles().toRGBA8(false);

		tex2d.writeFile(file.toStdString());
	});

	//TODO: import button, import convert

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

void TilemapEditor::setPalette(Texture2D tex) {
	renderer->setPalette(tex);
}

Texture2D TilemapEditor::getPalette() {
	return renderer->getPalette();
}

void TilemapEditor::setTilemap(Texture2D tex) {
	renderer->setTilemap(tex);
}

Texture2D TilemapEditor::getTilemap() {
	return renderer->getTilemap();
}

void TilemapEditor::setTiles(Texture2D tex) {
	renderer->setTiles(tex);
}

Texture2D TilemapEditor::getTiles() {
	return renderer->getTiles();
}

void TilemapEditor::setUsePalette(bool b) {
	renderer->setUsePalette(b);
}

bool TilemapEditor::getUsePalette() {
	return renderer->getUsePalette();
}

void TilemapEditor::setUseGrid(bool b) {
	renderer->setUseGrid(b);
}

bool TilemapEditor::getUseGrid() {
	return renderer->getUseGrid();
}

bool TilemapEditor::allowsResource(FileSystemObject&, ArchiveObject &ao) {
	return 
		ao.info.magicNumber == NCGR::getMagicNumber() || 
		ao.info.magicNumber == NCLR::getMagicNumber() || 
		ao.info.magicNumber == NSCR::getMagicNumber();
}

void TilemapEditor::inspectResource(FileSystem &fileSystem, FileSystemObject &fso, ArchiveObject &ao) {

	if (ao.info.magicNumber == NCGR::getMagicNumber()) {

		NCGR &ncgr = fileSystem.get<NCGR>(ao);

		setTiles(Texture2D(ncgr));

		tile = &ncgr;
		tileName = QString::fromStdString(fso.name);

	} 
	else if (ao.info.magicNumber == NSCR::getMagicNumber()) {

		NSCR &nscr = fileSystem.get<NSCR>(ao);

		setTilemap(Texture2D(nscr));

		tilemap = &nscr;
		tilemapName = QString::fromStdString(fso.name);

	} else {

		NCLR &nclr = fileSystem.get<NCLR>(ao);

		setPalette(Texture2D(nclr));

		palette = &nclr;
		paletteName = QString::fromStdString(fso.name);
	}
}

bool TilemapEditor::isPrimaryEditor(FileSystemObject&, ArchiveObject &ao) {
	return ao.info.magicNumber == NSCR::getMagicNumber();
}

void TilemapEditor::onSwap() {
	renderer->updateTexture();
}

void TilemapEditor::reset() {
	renderer->reset();
	palette = nullptr;
	tile = nullptr;
	tilemap = nullptr;
	paletteName = "";
	tilemapName = "";
	tileName = "";
	repaint();
}

enum Property : u32 {

	NAME,
	WIDTH,
	HEIGHT,
	TILE_NAME,
	TILE_WIDTH,
	TILE_HEIGHT,
	TILE_TILING,
	TILE_BIT_DEPTH,
	TILE_ENCRYPTION,
	TILE_UNDEFINED,
	PALETTE_NAME,
	COLORS,
	PALETTES,

	END,
	START = NAME
};

static const c8 *properties[] = {
	"Name",
	"Width",
	"Height",
	"Tile name",
	"Tile width",
	"Tile height",
	"Tile tiling",
	"Tile bit depth",
	"Tile encryption",
	"Tile undefined",
	"Palette name",
	"Colors",
	"Palettes",
};

inline void set(InfoWindow *info, Property prop, const QString &str) {
	info->setString(properties[prop], str);
}

void TilemapEditor::showInfo(InfoWindow *info) {

	set(info, NAME, tilemapName);

	if (tilemap != nullptr && renderer->getTilemap().getWidth() != 0) {
		set(info, WIDTH, QString::number(renderer->getTilemap().getWidth()));
		set(info, HEIGHT, QString::number(renderer->getTilemap().getHeight()));
	}

	set(info, TILE_NAME, tileName);

	if (tile != nullptr && renderer->getTiles().getWidth() != 0) {

		set(info, TILE_WIDTH, QString::number(renderer->getTiles().getWidth()));
		set(info, TILE_HEIGHT, QString::number(renderer->getTiles().getHeight()));
		set(info, TILE_TILING, QString::number(renderer->getTiles().getTiles()));
		set(info, TILE_BIT_DEPTH, QString::number(renderer->getTiles().getBitsPerPixel()));
		set(info, TILE_ENCRYPTION, tile->at<0>().isEncrypted ? "On" : "Off");

		set(info, TILE_UNDEFINED,
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

void TilemapEditor::hideInfo(InfoWindow *info) {

	for (u32 i = START; i != END; ++i)
		info->clearString(properties[i]);
}