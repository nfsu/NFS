#pragma once
#include <qabstractitemmodel.h>
#include <FileSystem.h>
#include <qevent.h>
#include <qtreeview.h>
#include "InfoTable.h"

#include "NEditors.h"

class NExplorer : public QAbstractItemModel {

	friend class NExplorerView;

public:

	explicit NExplorer(u8 *begin, nfs::FileSystem &fs, QObject *parent = 0);

	QVariant data(const QModelIndex &index, int role) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:

	u8 *begin;
	nfs::FileSystem &fs;
	u32 fileCount;
};

class NExplorerView : public QTreeView {

public:

	NExplorerView(NExplorer *_nex, InfoTable *_fileInfo, NEditors *_editors) : fso(nullptr), nex(_nex), fileInfo(_fileInfo), editors(_editors) {
		setUniformRowHeights(true);
		setModel(nex);

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, &QTreeView::customContextMenuRequested, this, &NExplorerView::onCustomContextMenu);
	}
	
	bool hasCurrent() { return fso != nullptr; }
	const nfs::FileSystemObject &getCurrent() { return *fso; }

	private slots:
	void onCustomContextMenu(const QPoint &point) {
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
		}
	}

private:

	nfs::FileSystemObject *fso;
	NExplorer *nex;
	InfoTable *fileInfo;
	NEditors *editors;
};