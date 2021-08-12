#pragma once

#include "file_system.hpp"

#pragma warning(push, 0)
	#include <QtWidgets/qtreeview.h>
#pragma warning(pop)

#include <functional>

namespace nfsu {

	//File explorer
	class NExplorer : public QAbstractItemModel {

	public:

		NExplorer(nfs::FileSystem &fs, const Map<String, String> &icons = {
			{ "", "resources/folder.png" },
			{ "TXT", "resources/text.png" },
			{ "SDAT", "resources/sound.png" },
			{ "NCLR", "resources/palette.png" },
			{ "NCGR", "resources/tilemap.png" },
			{ "NSCR", "resources/map.png" },
			{ "BMD0", "resources/model.png" },
			{ "NARC", "resources/archive.png" },
			{ "NONE", "resources/binary.png" }
		});

		QVariant data(const QModelIndex &index, int role) const override;
		Qt::ItemFlags flags(const QModelIndex &index) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex &index) const override;
		int rowCount(const QModelIndex &parent = QModelIndex()) const override;
		int columnCount(const QModelIndex &parent = QModelIndex()) const override;

		nfs::FileSystemObject *getRoot() const;
		nfs::FileSystem &getFileSystem() const;

		QPixmap getIcon(const String &ext) const;

	private:

		nfs::FileSystem &fs;
		Map<String, QPixmap> icons;
	};

	typedef std::function<void (nfs::FileSystem&, nfs::FileSystemObject&, const QPoint&)> ExplorerCallback;
	typedef std::function<void(nfs::FileSystem&, nfs::FileSystemObject&, nfs::ArchiveObject&, const QPoint&)> ResourceCallback;

	//File explorer view
	class NExplorerView : public QTreeView {

	public:

		NExplorerView(NExplorer *explorer);

		inline nfs::FileSystemObject *getCurrent() const { return current; }

		void customContextMenuRequested(const QPoint &point);
		void onLeftClick(const QModelIndex &index);

	public:

		//Add an action on right click of an fso
		void addExplorerCallback(bool isRightClick, ExplorerCallback callback);

		//Add an action on right click of a resource (overrides current type callback)
		void addResourceCallback(bool isRightClick, u32 type, ResourceCallback callback);

	private:

		nfs::FileSystemObject *current;
		NExplorer *exp;

		struct NExViewCallback {
			List<ExplorerCallback> ecall;
			Map<u32, ResourceCallback> rcall;
		};

		Map<bool, NExViewCallback> callbacks;
	};
}