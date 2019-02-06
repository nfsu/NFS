#include "paletteeditor.h"
#include "paletterenderer.h"
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qcolordialog.h>
using namespace nfsu;
using namespace nfs;

PaletteEditor::PaletteEditor(u32 scale, QWidget *parent) : QWidget(parent) {

	QHBoxLayout *layout = new QHBoxLayout;
	setLayout(layout);

	layout->addWidget(renderer = new PaletteRenderer);

	setFixedSize(scale * 16, scale * 16);

	//TODO: Button for showing / disabling grid, grid color and size
	//TODO: Show properties of palette
	//TODO: onResize

}

void PaletteEditor::setPalette(Texture2D tex) {
	renderer->setTexture(tex);
}

Texture2D PaletteEditor::getPalette() {
	return renderer->getTexture();
}

PaletteRenderer *PaletteEditor::getRenderer() {
	return renderer;
}

bool PaletteEditor::allowsResource(FileSystemObject &fso, ArchiveObject &ao) {
	return ao.info.magicNumber == NCLR::getMagicNumber();
}

bool PaletteEditor::isPrimaryEditor(FileSystemObject &fso, ArchiveObject &ao) {
	return allowsResource(fso, ao);
}

void PaletteEditor::inspectResource(FileSystem &fileSystem, ArchiveObject &ao) {
	setPalette(Texture2D(fileSystem.get<NCLR>(ao)));
}

void PaletteEditor::onSwap() {
	renderer->updateTexture();
}