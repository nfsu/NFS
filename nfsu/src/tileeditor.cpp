#include "tileeditor.h"
#include "tilerenderer.h"
#include "infowindow.h"
#include "paletterenderer.h"
#include <QtGui/qevent.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qfiledialog.h>
using namespace nfsu;
using namespace nfs;

TileEditor::TileEditor() {

	//Renderer

	renderer = new TileRenderer;
	renderer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	//Buttons

	QWidget *palette = new QWidget;

	QGridLayout *gridLayout = new QGridLayout;

	QPushButton *exportButton = new QPushButton(QIcon(QPixmap("resources/export.png")), "");
	QPushButton *exportConvertButton = new QPushButton(QIcon(QPixmap("resources/export_converted.png")), "");
	QPushButton *importButton = new QPushButton(QIcon(QPixmap("resources/import.png")), "");
	QPushButton *importConvertButton = new QPushButton(QIcon(QPixmap("resources/import_converted.png")), "");
	QPushButton *paletteButton = new QPushButton(QIcon(QPixmap("resources/palette.png")), "");
	QPushButton *gridButton = new QPushButton(QIcon(QPixmap("resources/grid.png")), "");

	connect(paletteButton, &QPushButton::clicked, this, [&]() { this->setUsePalette(!this->getUsePalette()); });
	//connect(gridButton, &QPushButton::clicked, this, )

	connect(exportButton, &QPushButton::clicked, this, [&]() {

		QString filters;

		bool hasTiledData = this->renderer->getTexture().getWidth() != 0;
		bool hasPaletteData = this->renderer->getPalette().getWidth() != 0;

		if (this->tile)
			filters += "NCGR file (*.NCGR)";
		else if (hasTiledData)
			filters += "Tiled data (*.bin)";

		if (hasTiledData && hasPaletteData)
			filters += ";;";

		if (this->palette)
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
			buf = this->tile->toBuffer();
		else if (filter == "NCLR file (*.NCLR)")
			buf = this->palette->toBuffer();
		else if (filter == "Palette data (*.bin)")
			buf = this->renderer->getPalette().toBuffer();
		else
			buf = this->renderer->getTexture().toBuffer();

		ofile.write((const char*)buf.ptr, buf.size);
		ofile.close();

	});

	connect(exportConvertButton, &QPushButton::clicked, this, [&]() {

		QString filter;
		QString file = QFileDialog::getSaveFileName(nullptr, tr("Save converted resource"), "", "Converted file (*.png);;Palette file (*.png);;Tiled file (*.png)", &filter);

		if (file == "")
			return;

		bool hasTiledData = this->renderer->getTexture().getWidth() != 0;
		bool hasPaletteData = this->renderer->getPalette().getWidth() != 0;

		if ((!hasTiledData && !hasPaletteData) || !this->renderer->getUsePalette()) {
			QMessageBox messageBox;
			messageBox.critical(0, "Error", "Please select a palette and tiled image and enable the palette");
			messageBox.setFixedSize(500, 200);
			return;
		}

		Texture2D tex2d;

		if (filter == "Converted file (*.png)")
			tex2d = Texture2D(this->renderer->getTexture(), this->renderer->getPalette());
		else if (filter == "Palette file (*.png)")
			tex2d = this->renderer->getPalette().toRGBA8();
		else
			tex2d = this->renderer->getTexture().toRGBA8();

		tex2d.write(file.toStdString());

	});

	//TODO: Import button, import convert

	gridLayout->addWidget(importButton, 0, 0);
	gridLayout->addWidget(importConvertButton, 1, 0);
	gridLayout->addWidget(exportButton, 0, 1);
	gridLayout->addWidget(exportConvertButton, 1, 1);
	gridLayout->addWidget(gridButton, 0, 2);
	gridLayout->addWidget(paletteButton, 1, 2);

	gridLayout->setMargin(5);
	palette->setLayout(gridLayout);
	palette->setContentsMargins(0, 0, 0, 0);
	palette->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	//Layout

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(0);
	layout->setMargin(0);
	setLayout(layout);

	layout->addWidget(palette);
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

bool TileEditor::allowsResource(FileSystemObject &fso, ArchiveObject &ao) {
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

bool TileEditor::isPrimaryEditor(FileSystemObject &fso, ArchiveObject &ao) {
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

void TileEditor::showInfo(InfoWindow *info) {

	info->setString("Tile name", tileName);

	if (tile != nullptr && renderer->getTexture().getWidth() != 0) {

		info->setString("Width", QString::number(renderer->getTexture().getWidth()));
		info->setString("Height", QString::number(renderer->getTexture().getHeight()));
		info->setString("Tiling", QString::number(renderer->getTexture().getTiles()));
		info->setString("Bit depth", QString::number(renderer->getTexture().getBitsPerPixel()));
		info->setString("Encryption", tile->at<0>().isEncrypted ? "On" : "Off");

		info->setString("Special",
			QString::number(tile->at<0>().sizeHint0) + " " + QString::number(tile->at<0>().sizeHint1) + " " +
			QString::number(tile->at<0>().sizeHint2) + " " + QString::number(tile->at<0>().specialTiling)
		);


	} else {
		
		info->setString("Width", "");
		info->setString("Height", "");
		info->setString("Tiling", "");
		info->setString("Bit depth", "");
		info->setString("Encryption", "");
		info->setString("Special", "");

	}

	info->setString("Palette name", paletteName);

	if (palette != nullptr && renderer->getPalette().getWidth() != 0) {

		info->setString("Colors", QString::number(renderer->getPalette().getWidth()));
		info->setString("Palettes", QString::number(renderer->getPalette().getHeight()));

	} else {

		info->setString("Colors", "");
		info->setString("Palettes", "");

	}

}

void TileEditor::hideInfo(InfoWindow *info) {

	info->clearString("Tile name");

	info->clearString("Width");
	info->clearString("Height");
	info->clearString("Tiling");
	info->clearString("Bit depth");
	info->clearString("Encryption");
	info->clearString("Special");

	info->clearString("Palette name");
	info->clearString("Colors");
	info->clearString("Palettes");

}