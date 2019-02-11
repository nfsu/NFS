#include "tileeditor.h"
#include "tilerenderer.h"
#include "infowindow.h"
#include "paletterenderer.h"
#include <QtGui/qevent.h>
using namespace nfsu;
using namespace nfs;

TileEditor::TileEditor() {

	//Renderer
	renderer = new TileRenderer;
	renderer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	//Layout

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	layout->addWidget(renderer);

	//TODO: InfoWindow should have a scrollbar (vertical)
	//TODO: InfoWindow column size is invalid (should be stretch)
	//TODO: Separation between file explorer and editor data

	//TODO: Palette disable button, palette grid, tile grid
	//TODO: Lookup palette & tile button/file explorer
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

void TileEditor::usePalette(bool b) {
	renderer->setUsePalette(b);
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