#pragma once
#include <QtWidgets/qsplitter.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qgridlayout.h>
#include "texture.h"
#include "paletteeditor.h"

namespace nfsu {

	class TileRenderer;
	class InfoWindow;

	class TileEditor : public QWidget, public ResourceEditor {

	public:

		TileEditor();

		void setPalette(nfs::Texture2D tex);
		nfs::Texture2D getPalette();

		void setTiles(nfs::Texture2D tex);
		nfs::Texture2D getTiles();

		void usePalette(bool b);

		bool allowsResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;
		void inspectResource(nfs::FileSystem &fileSystem, nfs::ArchiveObject &ao) override;

		void onSwap() override;
		void reset() override;

		bool isPrimaryEditor(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;

	private:

		QGridLayout *rightLayout;
		TileRenderer *renderer;
		InfoWindow *info;

	};

}