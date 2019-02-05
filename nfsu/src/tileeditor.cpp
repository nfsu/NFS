#include "tileeditor.h"
#include "tilerenderer.h"
using namespace nfsu;
using namespace nfs;

TileEditor::TileEditor(u32 tileScale, u32 paletteScale, QWidget *parent): QSplitter(parent),
	scale(tileScale) {

	QSplitter *leftWidget = new QSplitter;
	addWidget(leftWidget);
	leftWidget->addWidget(palette = new PaletteEditor(paletteScale));

	addWidget(renderer = new TileRenderer(palette->getRenderer()));
	renderer->setFixedSize(512, 512);

	//TODO: Resize button (also allow disabling upscaling)
	//TODO: Palette disable button, palette grid, tile grid
	//TODO: Lookup palette & tile button/file explorer
	//TODO: Predict size; get a few sizes, if the border is 0 of an image, it probably is the correct size
	//TODO: Allow changing size, tool, color

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

bool TileEditor::allowsResource(ArchiveObject &ao) {
	return ao.info.magicNumber == NCGR::getMagicNumber() || palette->allowsResource(ao);
}

void TileEditor::inspectResource(FileSystem &fileSystem, ArchiveObject &ao) {

	if (ao.info.magicNumber == NCGR::getMagicNumber())
		setTiles(Texture2D(fileSystem.get<NCGR>(ao)));
	else {
		palette->inspectResource(fileSystem, ao);
		renderer->refresh();
	}

}