#pragma once
#include "archive.h"
#include "filesystem.h"

namespace nfsu {

	class ResourceEditor {

	public:

		//Whether or not the editor can display the file or resource type
		virtual bool allowsResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) = 0;

		//Handle displaying the resource internally
		virtual void inspectResource(nfs::FileSystem &fileSystem, nfs::ArchiveObject &ao) = 0;

		//Whenever the tab is switched to
		virtual void onSwap() = 0;

		//When the editor is reset (like ROM reload)
		virtual void reset() = 0;

		//Whether or not the editor is made specifically for the file or resource type
		virtual bool isPrimaryEditor(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) = 0;

		//Whether or not the editor can show pure data
		virtual bool allowsData() { return false; }

		//Handle displaying & editing data internally
		virtual void inspectData(Buffer data) {}

	};

}