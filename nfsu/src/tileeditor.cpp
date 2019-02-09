#include "tileeditor.h"
#include "tilerenderer.h"
#include "paletterenderer.h"
#include <QtGui/qevent.h>
using namespace nfsu;
using namespace nfs;

TileEditor::TileEditor(QWidget *parent): QSplitter(parent) {

	addWidget(renderer = new TileRenderer);

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

void TileEditor::inspectResource(FileSystem &fileSystem, ArchiveObject &ao) {

	if (ao.info.magicNumber == NCGR::getMagicNumber())
		setTiles(Texture2D(fileSystem.get<NCGR>(ao)));
	else 
		setPalette(Texture2D(fileSystem.get<NCLR>(ao)));

}

bool TileEditor::isPrimaryEditor(FileSystemObject &fso, ArchiveObject &ao) {
	return ao.info.magicNumber == NCGR::getMagicNumber();
}

void TileEditor::onSwap() {
	renderer->updateTexture();
}

void TileEditor::reset() {
	renderer->reset();
	repaint();
}