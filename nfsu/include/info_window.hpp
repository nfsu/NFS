#pragma once
#include "generic.hpp"

#pragma warning(push, 0)
	#include <QtWidgets/qtablewidget.h>
#pragma warning(pop)

class QLayout;
class QWidget;

namespace nfsu {

	class InfoWindow : public QTableWidget {

	public:

		InfoWindow(bool scrollHorizontal, bool scrollVertical, QWidget *parent = nullptr);
		~InfoWindow() {}

		void setString(QString key, QString value);
		void clearString(QString key);
		void reset();

		int sizeHintForColumn(int) const override;

	protected:

		void updateRow(u32 i, QString value);
		void updateHeight();

	private:

		QList<QPair<QString, QString>> table;
		QLayout *layout = nullptr;

		bool scrollHorizontal, scrollVertical;
	};
}