#include "infowindow.h"
#include "qhelper.h"
#include <QtWidgets/qlabel.h>
using namespace nfsu;

InfoWindow::InfoWindow(std::string str, QWidget *parent): QWidget(parent) { setFixedSize(QSize(300, 125)); }
InfoWindow::~InfoWindow() {}

void InfoWindow::clear() {
	QHelper::clearLayout(layout);
}

void InfoWindow::addString(QString str) {
	QLabel *label;
	layout->addWidget(label = new QLabel(str, this));
}