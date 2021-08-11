#include "info_window.hpp"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qscrollbar.h>
using namespace nfsu;

InfoWindow::InfoWindow(bool scrollHorizontal, bool scrollVertical, QWidget *parent):
	QTableWidget(parent), scrollHorizontal(scrollHorizontal), scrollVertical(scrollVertical) 
{
	setColumnCount(2);

	setHorizontalScrollBarPolicy(scrollHorizontal ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(scrollVertical ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);

	setSizePolicy(QSizePolicy::Expanding, scrollVertical ? QSizePolicy::MinimumExpanding : QSizePolicy::Expanding);

	verticalHeader()->setVisible(false);
	horizontalHeader()->setVisible(false);

	horizontalHeader()->setSectionResizeMode(1, scrollHorizontal ? QHeaderView::ResizeToContents : QHeaderView::Stretch);

	if (scrollVertical)
		verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	setHorizontalScrollMode(ScrollMode::ScrollPerPixel);
	setVerticalScrollMode(ScrollMode::ScrollPerPixel);
}

void InfoWindow::setString(QString key, QString value) {

	usz i = 0;

	for(auto &elem : table)

		if (elem.first == key) {
			elem.second = value;
			updateRow(i, value);
			return;
		} 

		else ++i;

	setRowCount(rowCount() + 1);

	table.push_back({ key, value });
	setItem(i, 0, new QTableWidgetItem(key));
	setItem(i, 1, new QTableWidgetItem(value));

	item(i, 0)->setFlags(Qt::ItemIsEnabled);
	item(i, 1)->setFlags(Qt::ItemIsEnabled);

	updateHeight();
}

void InfoWindow::clearString(QString key) {

	usz i = 0;

	for (auto &elem : table)

		if (elem.first == key) {
			table.removeOne(elem);
			removeRow(i);
			updateHeight();
			return;
		}

		else ++i;
}

int InfoWindow::sizeHintForColumn(int i) const {

	if(i == 0 || !scrollHorizontal)
		return QTableWidget::sizeHintForColumn(0);

	int viewSize = viewport()->width() - horizontalHeader()->sectionSize(0);
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

	i32 height = 
		verticalHeader()->count() * verticalHeader()->sectionSize(0) + 2 + 
		(scrollHorizontal ? horizontalScrollBar()->height() : 0);

	if (scrollVertical) {
		setMaximumHeight(height);
		return;
	}

	setFixedHeight(height);
}