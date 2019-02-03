#pragma once
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qgridlayout.h>
#include "texture.h"
#include "resourceeditor.h"

namespace nfsu {

	class PaletteEditor : public QWidget, public ResourceEditor {

	public:

		PaletteEditor(u32 w, u32 h, u32 scale, QWidget *parent = nullptr);

		void setPalette(nfs::Texture2D tex);
		void clearPalette();

		bool allowsResource(nfs::ArchiveObject &ao) override;
		void inspectResource(nfs::FileSystem &fileSystem, nfs::ArchiveObject &ao) override;

		nfs::Texture2D getPalette();

	private:

		u32 maxWidth, maxHeight;

		QGridLayout *layout;

		nfs::Texture2D paletteTexture;
		std::vector<QPushButton*> palette;

	};

}