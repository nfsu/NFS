#include "tileeditor.h"
#include "tilerenderer.h"
#include "infowindow.h"
#include "paletterenderer.h"
#include <QtGui/qevent.h>
using namespace nfsu;
using namespace nfs;

TileEditor::TileEditor() {

	//Info window

	info = new InfoWindow;

	info->setFixedWidth(200);
	info->setString("Width", "");
	info->setString("Height", "");
	info->setString("bpp", "");

	//Buttons

	//Top bar

	QWidget *bar = new QWidget;
	bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	bar->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout *barLayout = new QHBoxLayout;
	barLayout->setContentsMargins(2, 2, 2, 2);
	bar->setLayout(barLayout);

	barLayout->addWidget(info);
	barLayout->addStretch();
	//barLayout->addWidget(buttons);
	//barLayout->addWidget(palettePreview);
	//barLayout->addWidget(paletteButtons);

	//Renderer
	renderer = new TileRenderer;
	renderer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	//Layout

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	layout->addWidget(bar);
	layout->addWidget(renderer);

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

	if (ao.info.magicNumber == NCGR::getMagicNumber()) {

		NCGR &ncgr = fileSystem.get<NCGR>(ao);
		Texture2D tex = ncgr;

		setTiles(tex);

		info->setString("Width", QString::number(tex.getWidth()));
		info->setString("Height", QString::number(tex.getHeight()));
		info->setString("bpp", QString::number(tex.getBitsPerPixel()));

	} else 
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