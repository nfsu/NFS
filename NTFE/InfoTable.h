#pragma once
#include <qtableview.h>
#include <FileSystem.h>

class InfoTable : public QAbstractTableModel {

public:

	InfoTable(nfs::NDS &_nds, nfs::FileSystem &_fs, std::unordered_map<std::string, std::vector<std::string>> _contents, QObject *parent = 0);

	int rowCount(const QModelIndex &) const override;
	int columnCount(const QModelIndex &) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	QVariant data(const QModelIndex &index, int role) const override;

	void set(std::string where, u32 index, std::string value);

private:

	nfs::NDS &nds;
	nfs::FileSystem &fs;
	std::unordered_map<std::string, std::vector<std::string>> contents;
	std::vector<std::string> keys;
	u32 maxValues;
};