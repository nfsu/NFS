#pragma once
#include "resource_editor.hpp"

#pragma warning(push, 0)
	#include <QtWidgets/qsplitter.h>
#pragma warning(pop)

namespace nfsu {

	class TileEditor;

	class GameEditor : public QSplitter, public ResourceEditor {

	public:

		GameEditor();

		TileEditor *getLogoEditor();

		void onSwap() override;
		void init(nfs::NDS *nds, nfs::FileSystem &fs) override;
		void reset() override;

		void showInfo(InfoWindow *info) override;
		void hideInfo(InfoWindow *info) override;

	private:

		TileEditor *logoEditor;

		nfs::NDS *rom = nullptr;
		nfs::NDSBanner *bannerData;
	};
}