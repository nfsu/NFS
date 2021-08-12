#pragma once
#include "archive.hpp"
#include "file_system.hpp"

namespace nfsu {

	class InfoWindow;

	class ResourceEditor {

	public:

		//Whether or not the editor can display the file or resource type
		virtual bool allowsResource(nfs::FileSystemObject&, nfs::ArchiveObject&) { return false; }

		//Handle displaying the resource internally
		virtual void inspectResource(nfs::FileSystem&, nfs::FileSystemObject&, nfs::ArchiveObject&) {}

		//Whenever the tab is switched to
		virtual void onSwap() = 0;

		//Add info to info window
		virtual void showInfo(InfoWindow*) {};

		//Remove info from info window
		virtual void hideInfo(InfoWindow*) {};

		//When the editor is reset (like ROM reload)
		virtual void reset() = 0;

		//When the file system is initialized
		virtual void init(nfs::NDS*, nfs::FileSystem&) {}

		//Whether or not the editor is made specifically for the file or resource type
		virtual bool isPrimaryEditor(nfs::FileSystemObject&, nfs::ArchiveObject&) { return false; }

		//Whether or not the editor can show pure data
		virtual bool allowsData() { return false; }

		//Handle displaying & editing data internally
		virtual void inspectData(Buffer) {}

	};
}