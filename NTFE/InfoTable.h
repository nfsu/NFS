#pragma once
#include <qtableview.h>
#include <FileSystem.h>

class InfoTable : public QAbstractTableModel {

public:

	InfoTable(nfs::NDS &_nds, nfs::FileSystem &_fs, std::unordered_map<std::string, std::vector<std::string>> _contents, QObject *parent = 0) : QAbstractTableModel(parent), nds(_nds), fs(_fs), contents(_contents), maxValues(0) {
		keys.resize(contents.size());
		u32 i = 0;
		for (auto &elem : contents) {
			keys[i] = elem.first;
			if(elem.second.size() > maxValues)
				maxValues = elem.second.size();
			++i;
		}
	}

	int rowCount(const QModelIndex &) const override { return maxValues; }
	int columnCount(const QModelIndex &) const override { return keys.size(); }

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
			return keys[section].c_str();
		return QVariant();
	}

	QVariant data(const QModelIndex &index, int role) const override {
		if (role == Qt::DisplayRole) {
			auto vec = contents.at(keys[index.column()]);
			return vec.size() <= index.row() ? "" : vec[index.row()].c_str();
		}

		return QVariant();
	}

	void set(std::string where, u32 index, std::string value) {
		contents[where][index] = value;
	}

private:

	nfs::NDS &nds;
	nfs::FileSystem &fs;
	std::unordered_map<std::string, std::vector<std::string>> contents;
	std::vector<std::string> keys;
	u32 maxValues;
};