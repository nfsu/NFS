#include "NExplorer.h"
#include <qcolor.h>
#include <qapplication.h>
#include <qpalette.h>
#include <qevent.h>
using namespace nfs;

NExplorer::NExplorer(u8 *_begin, FileSystem &_fs, QObject *parent) : QAbstractItemModel(parent), begin(_begin), fs(_fs) { }

int NExplorer::columnCount(const QModelIndex &parent) const { return 1; }
QVariant NExplorer::headerData(int section, Qt::Orientation orientation, int role) const {

	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) 
			return QString("Files");

	return QVariant();
}

QVariant NExplorer::data(const QModelIndex &index, int role) const {

	if (!index.isValid())
		return QVariant();

	if (role == Qt::BackgroundRole) {
		int batch = (index.row() / 100) % 2;
		QApplication *app = (QApplication*)QApplication::instance();
		if (batch == 0)
			return app->palette().base();
		else
			return app->palette().base();
	}

	if (role != Qt::DisplayRole && role != Qt::DecorationRole)
		return QVariant();


	const FileSystemObject &var = *static_cast<const FileSystemObject*>(index.internalPointer());

	u32 magicNumber;
	std::string name;
	bool valid = var.getMagicNumber(name, magicNumber);


	if (role == Qt::DecorationRole)
		if (var.isFolder())
			return QPixmap(QString("Resources/Folder.png"));
		else if (name == "NARC")
			return QPixmap(QString("Resources/Archive.png"));
		else if (name == "TXT")
			return QPixmap(QString("Resources/Text.png"));
		else if (name == "SDAT")
			return QPixmap(QString("Resources/Sound.png"));
		else if (name == "NCLR")
			return QPixmap(QString("Resources/Palette.png"));
		else if (name == "NSCR")
			return QPixmap(QString("Resources/Map.png"));
		else if (name == "BMD0")
			return QPixmap(QString("Resources/Model.png"));
		else
			return QPixmap(QString("Resources/Binary.png"));

	return QString(var.name.c_str());
}

QModelIndex NExplorer::index(int row, int column, const QModelIndex &parent) const {

	u32 index = 0;

	if (parent.isValid())
		index = static_cast<const FileSystemObject*>(parent.internalPointer())->index;

	auto something = fs[fs[index]];

	if (row >= something.size())
		return QModelIndex();

	return createIndex(row, column, (void*)something[row]);
}

QModelIndex NExplorer::parent(const QModelIndex &index) const {

	if (!index.isValid())
		return QModelIndex();

	const FileSystemObject &child = *static_cast<const FileSystemObject*>(index.internalPointer());
	const FileSystemObject &father = fs[child.parent];

	if (father.isRoot())
		return QModelIndex();

	auto contents = fs[fs[father.parent]];

	u32 row = 0;
	for (u32 i = 0; i < contents.size(); ++i)
		if (contents[i]->index == father.index) {
			row = i;
			break;
		}

	return createIndex((int)row, 0, (void*)&father);
}

int NExplorer::rowCount(const QModelIndex &parent) const {

	u32 index = 0;

	if (parent.column() > 0)
		return 0;

	if (parent.isValid())
		index = static_cast<const FileSystemObject*>(parent.internalPointer())->index;

	auto something = fs[fs[index]];
	return (int)something.size();
}

Qt::ItemFlags NExplorer::flags(const QModelIndex &index) const {

	if (!index.isValid())
		return 0;

	return QAbstractItemModel::flags(index);
}

NExplorerView::NExplorerView(NExplorer *_nex, InfoTable *_fileInfo, NEditors *_editors) : fso(nullptr), nex(_nex), fileInfo(_fileInfo), editors(_editors) {
	setUniformRowHeights(true);
	setModel(nex);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QTreeView::customContextMenuRequested, this, &NExplorerView::onCustomContextMenu);
}

bool NExplorerView::hasCurrent() { return fso != nullptr; }
const nfs::FileSystemObject &NExplorerView::getCurrent() { return *fso; }

void NExplorerView::onCustomContextMenu(const QPoint &point) {
	QModelIndex index = indexAt(point);

	if (index.isValid()) {
		fso = (nfs::FileSystemObject*)index.internalPointer();

		u32 magicNumber = 0;
		std::string name = "";
		bool valid = fso->getMagicNumber(name, magicNumber);

		fileInfo->set("Values", 5, QString::number(fso->index).toStdString());
		fileInfo->set("Values", 6, fso->isFolder() ? "" : name);
		fileInfo->set("Values", 7, fso->path);
		fileInfo->set("Values", 8, fso->isFolder() ? "" : QString::number(fso->buffer.data - nex->begin, 16).toStdString());
		fileInfo->set("Values", 9, fso->isFolder() ? "" : QString::number(fso->buffer.size).toStdString());

		emit fileInfo->dataChanged(QModelIndex(), QModelIndex());

		if (name == "NCLR") {
			Texture2D tex;
			nfs::NType::convert(nex->fs.get<nfs::NCLR>(fso->resource), &tex);

			editors->setTexture(0, tex);
		}
		else if (name == "NCGR") {
			Texture2D tex;
			nfs::NCGR ncgr = nex->fs.get<nfs::NCGR>(fso->resource);
			nfs::NType::convert(ncgr, &tex);

			editors->setTexture(1, tex);
		}
		else if (name == "NSCR") {
			Texture2D tex;
			nfs::NCSR ncgr = nex->fs.get<nfs::NCSR>(fso->resource);
			nfs::NType::convert(ncgr, &tex);

			editors->setTexture(2, tex);
		}
	}
}