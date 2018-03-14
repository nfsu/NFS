#include "qhelper.h"
#include <QtWidgets/qwidget.h>

void QHelper::clearLayout(QLayout *layout) {

	if (layout == nullptr) return;

	while (QLayoutItem *item = layout->takeAt(0)) {

		if (QWidget *widget = item->widget())
			widget->deleteLater();

		if (QLayout *childLayout = item->layout())
			clearLayout(childLayout);

		delete item;
	}

	delete layout;
}