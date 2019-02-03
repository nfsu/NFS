#include "nexplorer.h"
#include <QtWidgets/qapplication.h>
using namespace nfsu;
using namespace nfs;

NExplorer::NExplorer(FileSystem &_fs, std::unordered_map<std::string, std::string> _icons) : fs(_fs), icons(_icons.size()) {
	for (auto &a : _icons)
		icons[a.first] = QPixmap(a.second.c_str());
}

NExplorerView::NExplorerView(NExplorer *explorer) : exp(explorer), current(explorer == nullptr ? nullptr : explorer->getRoot()) {
	setUniformRowHeights(true);
	setModel(explorer);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QTreeView::customContextMenuRequested, this, &NExplorerView::customContextMenuRequested);
	connect(this, &QTreeView::clicked, this, &NExplorerView::onLeftClick);
}

FileSystem &NExplorer::getFileSystem() const { return fs; }
FileSystemObject *NExplorer::getRoot() const {
	if (fs.size() == 0) return nullptr;
	return &fs[0];
}

int NExplorer::columnCount(const QModelIndex &parent) const { return 1; }
QVariant NExplorer::headerData(int section, Qt::Orientation orientation, int role) const {

	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return QString("File");

	return QVariant();
}

QPixmap NExplorer::getIcon(std::string ext) const {
	auto it = icons.find(ext);

	if (it == icons.end())
		return icons.find("none")->second;

	return it->second;
}

QVariant NExplorer::data(const QModelIndex &index, int role) const {

	if (!index.isValid())
		return QVariant();

	if (role == Qt::BackgroundRole)
		return QVariant();

	if (role != Qt::DisplayRole && role != Qt::DecorationRole)
		return QVariant();

	FileSystemObject &fso = *(FileSystemObject*)index.internalPointer();
	ArchiveObject *ao = fso.isFile() ? &fs.getResource(fso) : nullptr;

	if (role == Qt::DecorationRole) {
		std::string ext = fso.isFolder() ? "" : (QString(ao->name.c_str()).split(".").last()).toLower().toStdString();
		return getIcon(ext);
	}

	return QString(fso.name.c_str()).split("/").last();
}

bool isIndex(FileSystem &fs, FileSystemObject &fso, u32 i, u32 index, u16 folders, u32 value) {

	if (fso.isFile())
		i = i + (u32)folders;

	return i == value;
}

QModelIndex NExplorer::index(int row, int column, const QModelIndex &parent) const {

	FileSystemObject *fso = parent.isValid() ? (FileSystemObject*)parent.internalPointer() : getRoot();

	if (fso == nullptr)
		return QModelIndex();

	FileSystemObject *child = fs.foreachInFolder(isIndex, *fso, fso->folders, (u32) row);

	if (child == nullptr)
		return QModelIndex();

	return createIndex(row, column, child);
}

QModelIndex NExplorer::parent(const QModelIndex &index) const {

	if (!index.isValid())
		return QModelIndex();

	FileSystemObject &fso = *(FileSystemObject*)index.internalPointer();
	FileSystemObject &parent = fs[fso.parent];

	if (parent.isRoot())
		return QModelIndex();

	return createIndex((int)parent.index, 0, &parent);
}

int NExplorer::rowCount(const QModelIndex &parent) const {

	if(parent.column() > 0)
		return 0;

	if (parent.isValid())
		return ((FileSystemObject*)parent.internalPointer())->objects;
	
	return fs.size() > 0 ? fs[0].objects : 0;
}

Qt::ItemFlags NExplorer::flags(const QModelIndex &index) const {

	if (!index.isValid())
		return 0;

	return QAbstractItemModel::flags(index);
}

nfs::FileSystemObject *NExplorerView::getCurrent() { return current; }

void NExplorerView::addExplorerCallback(bool isRightClick, ExplorerCallback callback) { callbacks[isRightClick].ecall.push_back(callback); }
void NExplorerView::addResourceCallback(bool isRightClick, u32 type, ResourceCallback callback) { callbacks[isRightClick].rcall[type] = callback; }

void NExplorerView::customContextMenuRequested(const QPoint &point) {

	QModelIndex index = indexAt(point);

	if (index.isValid()) {

		current = (FileSystemObject*) index.internalPointer();

		FileSystem &fs = exp->getFileSystem();
		FileSystemObject &fso = *current;
		ArchiveObject *ao = fso.isFile() ? &fs.getResource(fso) : nullptr;

		for (auto a : callbacks[true].ecall)
			a(fs, fso, point);

		if(ao != nullptr)
		for (auto &a : callbacks[true].rcall) {

			if (a.first != ao->info.type && a.first != u32_MAX)
				continue;

			a.second(fs, fso, *ao, point);
		}
	}
}

void NExplorerView::onLeftClick(const QModelIndex &index) {

	if (index.isValid()) {

		FileSystemObject *current = (FileSystemObject*)index.internalPointer();
		FileSystem &fs = exp->getFileSystem();
		FileSystemObject &fso = *current;
		ArchiveObject *ao = fso.isFile() ? &fs.getResource(fso) : nullptr;

		QPoint point(-1, -1);

		for (auto a : callbacks[false].ecall)
			a(fs, fso, point);

		if (ao != nullptr)
			for (auto &a : callbacks[false].rcall) {

				if (a.first != ao->info.type && a.first != u32_MAX)
					continue;

				a.second(fs, fso, *ao, point);
			}
	}
}