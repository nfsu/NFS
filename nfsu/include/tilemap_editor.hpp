#pragma once
#include "texture.hpp"
#include "resource_editor.hpp"

#pragma warning(push, 0)
	#include <QtWidgets/qwidget.h>
#pragma warning(pop)

namespace nfsu {

	class TilemapRenderer;

	class TilemapEditor : public QWidget, public ResourceEditor {

	public:

		TilemapEditor();

		void setPalette(nfs::Texture2D tex);
		nfs::Texture2D getPalette();

		void setTiles(nfs::Texture2D tex);
		nfs::Texture2D getTiles();

		void setTilemap(nfs::Texture2D tex);
		nfs::Texture2D getTilemap();

		void setUsePalette(bool b);
		bool getUsePalette();

		void setUseGrid(bool b);
		bool getUseGrid();

		bool allowsResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;

		void inspectResource(
			nfs::FileSystem &fileSystem, nfs::FileSystemObject &fso, nfs::ArchiveObject &ao
		) override;

		void onSwap() override;
		void reset() override;

		bool isPrimaryEditor(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;

		void showInfo(InfoWindow *info) override;
		void hideInfo(InfoWindow *info) override;

	private:

		TilemapRenderer *renderer;

		nfs::NSCR *tilemap = nullptr;
		nfs::NCGR *tile = nullptr;
		nfs::NCLR *palette = nullptr;

		QString tilemapName, tileName, paletteName;
	};
}