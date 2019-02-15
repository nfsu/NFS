#pragma once
#include <QtWidgets/qsplitter.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qgridlayout.h>
#include "texture.h"
#include "paletteeditor.h"

namespace nfsu {

	class TileRenderer;

	class TileEditor : public QWidget, public ResourceEditor {

	public:

		TileEditor();

		void setPalette(nfs::Texture2D tex);
		nfs::Texture2D getPalette();

		void setTiles(nfs::Texture2D tex);
		nfs::Texture2D getTiles();

		void setUsePalette(bool b);
		bool getUsePalette();

		bool allowsResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;
		void inspectResource(nfs::FileSystem &fileSystem, nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;

		void onSwap() override;
		void reset() override;

		bool isPrimaryEditor(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;

		void showInfo(InfoWindow *info) override;
		void hideInfo(InfoWindow *info) override;

	private:

		TileRenderer *renderer;

		nfs::NCGR *tile = nullptr;
		nfs::NCLR *palette = nullptr;

		QString tileName, paletteName;

	};

}