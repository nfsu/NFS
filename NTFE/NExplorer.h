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

	void setFlag(u32 flag);

private:

	u8 *begin;
	nfs::FileSystem &fs;
	u32 fileCount;

	u32 flag;
};

class NExplorerView : public QTreeView {

public:

	NExplorerView(NExplorer *_nex, InfoTable *_fileInfo, NEditors *_editors);
	
	bool hasCurrent();
	const nfs::FileSystemObject &getCurrent();

	private slots:
	void onCustomContextMenu(const QPoint &point);

private:

	nfs::FileSystemObject *fso;
	NExplorer *nex;
	InfoTable *fileInfo;
	NEditors *editors;
};