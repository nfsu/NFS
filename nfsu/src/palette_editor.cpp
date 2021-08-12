#include "palette_editor.hpp"
#include "info_window.hpp"

#pragma warning(push, 0)
	#include <QtWidgets/qgridlayout.h>
	#include <QtWidgets/qcolordialog.h>
	#include <QtGui/qevent.h>
#pragma warning(pop)

using namespace nfsu;
using namespace nfs;

//TODO: Button for showing / disabling grid, grid size
//TODO: Export resource, export texture, import resource, import texture, detect palette from texture
//TODO: Layout; horizontal bar, w dropdown for palette editing

void PaletteEditor::setPalette(Texture2D tex) {
	setTexture(tex);
}

Texture2D PaletteEditor::getPalette() {
	return getTexture();
}

PaletteRenderer *PaletteEditor::getRenderer(){
	return (PaletteRenderer*) this;
}

bool PaletteEditor::allowsResource(FileSystemObject&, ArchiveObject &ao) {
	return ao.info.magicNumber == NCLR::getMagicNumber();
}

bool PaletteEditor::isPrimaryEditor(FileSystemObject &fso, ArchiveObject &ao) {
	return allowsResource(fso, ao);
}

void PaletteEditor::inspectResource(FileSystem &fileSystem, FileSystemObject &fso, ArchiveObject &ao) {
	paletteName = QString::fromStdString(fso.name);
	setPalette(Texture2D(fileSystem.get<NCLR>(ao)));
}

void PaletteEditor::onSwap() {
	updateTexture();
}

void PaletteEditor::reset() {
	paletteName = "";
	PaletteRenderer::reset();
}

void PaletteEditor::showInfo(InfoWindow *info) {

	info->setString("Palette name", paletteName);

	if (getGPUTexture() != nullptr && getTexture().getWidth() != 0) {
		info->setString("Colors", QString::number(getTexture().getWidth()));
		info->setString("Palettes", QString::number(getTexture().getHeight()));
	} else {
		info->setString("Colors", "");
		info->setString("Palettes", "");
	}
}

void PaletteEditor::hideInfo(InfoWindow *info) {
	info->clearString("Palette name");
	info->clearString("Colors");
	info->clearString("Palettes");
}