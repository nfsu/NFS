#include "tileeditor.h"
#include "tilerenderer.h"
#include "paletterenderer.h"
#include <QtGui/qevent.h>
using namespace nfsu;
using namespace nfs;

TileEditor::TileEditor(u32 tileScale, u32 paletteScale, QWidget *parent): QSplitter(parent),
	scale(tileScale) {

	QSplitter *leftWidget = new QSplitter;
	addWidget(leftWidget);
	leftWidget->addWidget(palette = new PaletteEditor);
	palette->setMinimumSize(256, 256);

	addWidget(renderer = new TileRenderer(palette->getRenderer()));

	//TODO: Palette disable button, palette grid, tile grid
	//TODO: Lookup palette & tile button/file explorer
	//TODO: Allow changing size, tool, color
	//TODO: Shortcuts (like: Alt + click = eyedropper, Ctrl + mouse wheel = zoom, Ctrl + drag = move)

}

void TileEditor::setPalette(Texture2D tex) {
	palette->setPalette(tex);
}

Texture2D TileEditor::getPalette() {
	return palette->getPalette();
}

void TileEditor::setTiles(Texture2D tex) {
	renderer->setTexture(tex);
}

void TileEditor::usePalette(bool b) {
	renderer->usePalette(b);
}

Texture2D TileEditor::getTiles() {
	return renderer->getTexture();
}

bool TileEditor::allowsResource(FileSystemObject &fso, ArchiveObject &ao) {
	return ao.info.magicNumber == NCGR::getMagicNumber() || palette->allowsResource(fso, ao);
}

void TileEditor::inspectResource(FileSystem &fileSystem, ArchiveObject &ao) {

	if (ao.info.magicNumber == NCGR::getMagicNumber())
		setTiles(Texture2D(fileSystem.get<NCGR>(ao)));
	else {
		palette->inspectResource(fileSystem, ao);
		renderer->updateTexture();
	}

}

bool TileEditor::isPrimaryEditor(FileSystemObject &fso, ArchiveObject &ao) {
	return ao.info.magicNumber == NCGR::getMagicNumber();
}

void TileEditor::onSwap() {
	palette->getRenderer()->updateTexture();
	renderer->updateTexture();
}

void TileEditor::reset() {
	renderer->reset();
	repaint();
}