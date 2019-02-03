#include "paletteeditor.h"
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qcolordialog.h>
using namespace nfsu;
using namespace nfs;

PaletteEditor::PaletteEditor(u32 w, u32 h, u32 scale, QWidget *parent) : 
	QWidget(parent), maxWidth(w), maxHeight(h), paletteTexture(nullptr, 0, 0, 0) {

	setLayout(layout = new QGridLayout);

	u32 j = maxWidth * maxHeight;
	palette.resize(j);

	for (u32 i = 0; i < j; ++i) {

		auto *button = palette[i] = new QPushButton;

		layout->addWidget(button, i / maxWidth, i % maxWidth);
		button->setFixedSize(scale, scale);

		connect(button, &QPushButton::clicked, this, [&]() {

			QPushButton *button = (QPushButton*) sender();

			u32 ind = layout->indexOf(button);
			u32 i = ind % maxWidth;
			u32 j = ind / maxWidth;

			if (i >= paletteTexture.getWidth() || j >= paletteTexture.getHeight())
				return;

			u32 argb = paletteTexture.read(i, j);
			QColor color = QColor((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, argb & 0xFF);

			QColorDialog dialog(color);
			dialog.setWindowTitle("Pick color");
			dialog.exec();

			QColor pcolor = dialog.selectedColor();

			if (!pcolor.isValid())
				return;

			paletteTexture.write(i, j, (pcolor.blue() << 16) | (pcolor.green() << 8) | pcolor.red());

			argb = paletteTexture.read(i, j);
			color = QColor((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, argb & 0xFF);

			button->setStyleSheet(
				QString("background-color: ") + color.name() + ";"
				"margin-left: 2px;"
				"margin-right: 2px;"
				"margin-top: 2px;"
				"margin-bottom: 2px;"
			);

		});
	}

	clearPalette();

	setFixedSize(scale * maxWidth, scale * maxHeight);

}

void PaletteEditor::setPalette(Texture2D tex) {

	u32 j = maxWidth * maxHeight;

	for (u32 i = 0; i < j; ++i)
		palette[i]->setStyleSheet(
			"margin-left: 2px;"
			"margin-right: 2px;"
			"margin-top: 2px;"
			"margin-bottom: 2px;"
		);

	if (tex.getWidth() != 0 && tex.getHeight() != 0) {

		for(u32 i = 0; i < tex.getWidth() && i < maxWidth; ++i)
			for (j = 0; j < tex.getHeight() && j < maxHeight; ++j) {

				u32 k = j * maxWidth + i;
				u32 argb = tex.read(i, j);

				QColor color = QColor((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, argb & 0xFF);

				palette[k]->setStyleSheet(
					QString("background-color: ") + color.name() + ";"
					"margin-left: 2px;"
					"margin-right: 2px;"
					"margin-top: 2px;"
					"margin-bottom: 2px;"
				);

			}

	}

}

Texture2D PaletteEditor::getPalette() {
	return paletteTexture;
}

void PaletteEditor::clearPalette() {
	setPalette(paletteTexture = Texture2D(nullptr, 0, 0, 0));
}

bool PaletteEditor::allowsResource(ArchiveObject &ao) {
	return ao.info.magicNumber == NCLR::getMagicNumber();
}

void PaletteEditor::inspectResource(FileSystem &fileSystem, ArchiveObject &ao) {
	setPalette(paletteTexture = Texture2D(fileSystem.get<NCLR>(ao)));
}