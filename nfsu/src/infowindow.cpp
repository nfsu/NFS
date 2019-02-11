#include "infowindow.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qscrollbar.h>
using namespace nfsu;

InfoWindow::InfoWindow(bool useScrollbar, QWidget *parent):
	QTableWidget(parent), useScrollbar(useScrollbar) {

	setColumnCount(2);

	setHorizontalScrollBarPolicy(useScrollbar ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	verticalHeader()->setVisible(false);
	horizontalHeader()->setVisible(false);

	horizontalHeader()->setSectionResizeMode(1, useScrollbar ? QHeaderView::ResizeToContents : QHeaderView::Stretch);
	setHorizontalScrollMode(ScrollMode::ScrollPerPixel);

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

	updateHeight();

}

void InfoWindow::clearString(QString key) {

	u32 i = 0;

	for (auto &elem : table)
		if (elem.first == key) {
			table.removeOne(elem);
			removeRow(i);
			updateHeight();
			return;
		} else ++i;


}

int InfoWindow::sizeHintForColumn(int i) const {

	if(i == 0 || !useScrollbar)
		return QTableWidget::sizeHintForColumn(0);

	int viewSize = viewport()->width() - sizeHintForColumn(0) * 2 - 2;
	int colSize = QTableWidget::sizeHintForColumn(i);
	return qMax(viewSize, colSize);
}

void InfoWindow::reset() {
	for (auto &elem : table)
		elem.second = "";
}

void InfoWindow::updateRow(u32 i, QString value) {
	item(i, 1)->setText(value);
}

void InfoWindow::updateHeight() {
	i32 height = verticalHeader()->count() * verticalHeader()->sectionSize(0) + 2 + (useScrollbar ? horizontalScrollBar()->height() : 0);
	setFixedHeight(height);
}