#pragma once
#include "texture.hpp"
#include "palette_editor.hpp"

#pragma warning(push, 0)
	#include <QtWidgets/qwidget.h>
#pragma warning(pop)

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

		TileRenderer *renderer;

		nfs::NCGR *tile = nullptr;
		nfs::NCLR *palette = nullptr;

		QString tileName, paletteName;
	};
}