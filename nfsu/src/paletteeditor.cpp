#include "paletteeditor.h"
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qcolordialog.h>
#include <QtGui/qevent.h>
using namespace nfsu;
using namespace nfs;


//TODO: Button for showing / disabling grid, grid color and size
//TODO: Show properties of palette
//TODO: Export resource, export texture, import resource, import texture, detect palette from texture

void PaletteEditor::setPalette(Texture2D tex) {
	setTexture(tex);
}

Texture2D PaletteEditor::getPalette() {
	return getTexture();
}

PaletteRenderer *PaletteEditor::getRenderer(){
	return (PaletteRenderer*) this;
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
	updateTexture();
}

void PaletteEditor::reset() {
	PaletteRenderer::reset();
}