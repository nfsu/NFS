#include "InfoTable.h"

InfoTable::InfoTable(std::unordered_map<std::string, std::vector<std::string>> _contents, QObject *parent) : QAbstractTableModel(parent), contents(_contents), maxValues(0) {
	keys.resize(contents.size());
	u32 i = 0;
	for (auto &elem : contents) {
		keys[i] = elem.first;
		if (elem.second.size() > maxValues)
			maxValues = (u32)elem.second.size();
		++i;
	}
}

int InfoTable::rowCount(const QModelIndex &) const { return maxValues; }
int InfoTable::columnCount(const QModelIndex &) const { return (u32)keys.size(); }

QVariant InfoTable::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return keys[section].c_str();
	return QVariant();
}

QVariant InfoTable::data(const QModelIndex &index, int role) const {
	if (role == Qt::DisplayRole) {
		auto vec = contents.at(keys[index.column()]);
		return vec.size() <= index.row() ? "" : vec[index.row()].c_str();
	}

	return QVariant();
}

void InfoTable::set(std::string where, u32 index, std::string value) {
	contents[where][index] = value;
}