#pragma once
#include "archive.h"
#include "filesystem.h"

namespace nfsu {

	class ResourceEditor {

	public:

		virtual bool allowsResource(nfs::ArchiveObject &ao) = 0;
		virtual void inspectResource(nfs::FileSystem &fileSystem, nfs::ArchiveObject &ao) = 0;
		virtual void onSwap() = 0;

	};

}