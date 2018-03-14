#pragma once

#include <filesystem.h>
#include <QtWidgets/qtreeview.h>
#include <functional>

namespace nfsu {

	//File explorer
	class NExplorer : public QAbstractItemModel {

	public:

		NExplorer(nfs::FileSystem &fs, std::unordered_map<std::string, std::string> icons = {
			{ "", "resources/folder.png" },
			{ "txt", "resources/text.png" },
			{ "sdat", "resources/sound.png" },
			{ "nclr", "resources/palette.png" },
			{ "ncgr", "resources/tilemap.png" },
			{ "nscr", "resources/map.png" },
			{ "bmd0", "resources/model.png" },
			{ "narc", "resources/archive.png" },
			{ "none", "resources/binary.png" }
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

		QPixmap getIcon(std::string ext) const;

	private:

		nfs::FileSystem &fs;
		std::unordered_map<std::string, QPixmap> icons;
	};

	typedef std::function<void (nfs::FileSystem&, nfs::FileSystemObject&, const QPoint&)> ExplorerCallback;
	typedef std::function<void(nfs::FileSystem&, nfs::FileSystemObject&, nfs::ArchiveObject&, const QPoint&)> ResourceCallback;

	//File explorer view
	class NExplorerView : public QTreeView {

	public:

		NExplorerView(NExplorer *explorer);

		nfs::FileSystemObject *getCurrent();

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
			std::vector<ExplorerCallback> ecall;
			std::unordered_map<u32, ResourceCallback> rcall;
		};

		std::unordered_map<bool, NExViewCallback> callbacks;

	};
}