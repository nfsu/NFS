#pragma once
#include "archive.hpp"
#include "file_system.hpp"

namespace nfsu {

	class InfoWindow;

	class ResourceEditor {

	public:

		//Whether or not the editor can display the file or resource type
		virtual bool allowsResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) { return false; }

		//Handle displaying the resource internally
		virtual void inspectResource(
			nfs::FileSystem &fileSystem, nfs::FileSystemObject &fso, nfs::ArchiveObject &ao
		) {}

		//Whenever the tab is switched to
		virtual void onSwap() = 0;

		//Add info to info window
		virtual void showInfo(InfoWindow *info) {};

		//Remove info from info window
		virtual void hideInfo(InfoWindow *info) {};

		//When the editor is reset (like ROM reload)
		virtual void reset() = 0;

		//When the file system is initialized
		virtual void init(nfs::NDS *nds, nfs::FileSystem &fs) {}

		//Whether or not the editor is made specifically for the file or resource type
		virtual bool isPrimaryEditor(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) { return false; }

		//Whether or not the editor can show pure data
		virtual bool allowsData() { return false; }

		//Handle displaying & editing data internally
		virtual void inspectData(Buffer data) {}

	};
}