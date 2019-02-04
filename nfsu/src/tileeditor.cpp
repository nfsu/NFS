#include "tileeditor.h"
#include "tilerenderer.h"
using namespace nfsu;
using namespace nfs;

TileEditor::TileEditor(u32 tileScale, u32 paletteScale, QWidget *parent): QSplitter(parent),
	scale(tileScale) {

	QSplitter *leftWidget = new QSplitter;
	addWidget(leftWidget);
	leftWidget->addWidget(palette = new PaletteEditor(16, 16, paletteScale));

	addWidget(renderer = new TileRenderer(tileScale));

	//TODO: Resize button (also allow disabling upscaling)
	//TODO: Palette disable button
	//TODO: Lookup palette & tile button/file explorer
	//TODO: Predict size; get a few sizes, if the border is 0 of an image, it probably is the correct size
	//TODO: Somewhere there's a memory leak? Why is there 300 MiB allocated and not 128

}

void TileEditor::setPalette(Texture2D tex) {
	palette->setPalette(tex);
}

void TileEditor::clearPalette() {
	palette->clearPalette();
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

bool TileEditor::allowsResource(ArchiveObject &ao) {
	return ao.info.magicNumber == NCGR::getMagicNumber() || palette->allowsResource(ao);
}

void TileEditor::inspectResource(FileSystem &fileSystem, ArchiveObject &ao) {

	if (ao.info.magicNumber == NCGR::getMagicNumber())
		setTiles(Texture2D(fileSystem.get<NCGR>(ao)));
	else
		palette->inspectResource(fileSystem, ao);

}