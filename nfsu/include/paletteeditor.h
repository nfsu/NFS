#pragma once
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qgridlayout.h>
#include "texture.h"
#include "resourceeditor.h"

namespace nfsu {

	class PaletteRenderer;

	class PaletteEditor : public QWidget, public ResourceEditor {

	public:

		PaletteEditor(u32 scale, QWidget *parent = nullptr);

		void setPalette(nfs::Texture2D tex);
		nfs::Texture2D getPalette();

		PaletteRenderer *getRenderer();

		bool allowsResource(nfs::ArchiveObject &ao) override;
		void inspectResource(nfs::FileSystem &fileSystem, nfs::ArchiveObject &ao) override;

	private:

		PaletteRenderer *renderer;

	};

}