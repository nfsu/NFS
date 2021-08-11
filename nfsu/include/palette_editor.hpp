#pragma once
#include "texture.hpp"
#include "resource_editor.hpp"
#include "palette_renderer.hpp"

namespace nfsu {

	class PaletteRenderer;

	class PaletteEditor : public PaletteRenderer, public ResourceEditor {

	public:

		void setPalette(nfs::Texture2D tex);
		nfs::Texture2D getPalette();

		PaletteRenderer *getRenderer();

		bool allowsResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;
		void inspectResource(nfs::FileSystem &fileSystem, nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;

		void onSwap() override;
		void reset() override;
		
		bool isPrimaryEditor(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;

		void showInfo(InfoWindow *info) override;
		void hideInfo(InfoWindow *info) override;

	private:

		QString paletteName;
	};
}