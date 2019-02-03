#pragma once
#include <QtWidgets/qwidget.h>

namespace nfsu {

	class PaletteEditor : public QWidget {

	public:

		PaletteEditor(QWidget *parent = nullptr);

	private:

		QLayout *layout;

	};

}