#pragma once
#include "texture.h"
#include "resourceeditor.h"
#include "paletterenderer.h"

namespace nfsu {

	class PaletteRenderer;

	class PaletteEditor : public PaletteRenderer, public ResourceEditor {

	public:

		void setPalette(nfs::Texture2D tex);
		nfs::Texture2D getPalette();

		PaletteRenderer *getRenderer();

		bool allowsResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;
		void inspectResource(nfs::FileSystem &fileSystem, nfs::ArchiveObject &ao) override;

		void onSwap() override;
		void reset() override;
		
		bool isPrimaryEditor(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) override;

	};

}