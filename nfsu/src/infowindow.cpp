#include "infowindow.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qscrollbar.h>
using namespace nfsu;

InfoWindow::InfoWindow(QWidget *parent): 
	QTableWidget(parent) {

	setColumnCount(2);

	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	verticalHeader()->setVisible(false);
	horizontalHeader()->setVisible(false);

	horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

}

InfoWindow::~InfoWindow() {}

void InfoWindow::setString(QString key, QString value) {

	u32 i = 0;

	for(auto &elem : table)
		if (elem.first == key) {
			elem.second = value;
			updateRow(i, value);
			return;
		} else ++i;

	setRowCount(rowCount() + 1);

	table.push_back({ key, value });
	setItem(i, 0, new QTableWidgetItem(key));
	setItem(i, 1, new QTableWidgetItem(value));

	item(i, 0)->setFlags(Qt::ItemIsEnabled);
	item(i, 1)->setFlags(Qt::ItemIsEnabled);

	i32 height = verticalHeader()->count() * verticalHeader()->sectionSize(0) + 2;
	setFixedHeight(height);

}

void InfoWindow::updateRow(u32 i, QString value) {
	item(i, 1)->setText(value);
}