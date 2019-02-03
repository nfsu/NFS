#pragma once

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qtablewidget.h>
#include "generic.h"

namespace nfsu {

	class InfoWindow : public QTableWidget {

	public:

		InfoWindow(QWidget *parent = nullptr);
		~InfoWindow();

		void clear();
		void setString(QString key, QString value);

	protected:

		void updateRow(u32 i, QString value);

	private:

		QList<QPair<QString, QString>> table;
		QLayout *layout = nullptr;

	};

}