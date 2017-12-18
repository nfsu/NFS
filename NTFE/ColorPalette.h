#pragma once

#include <qwidget.h>
#include <Types.h>
#include <qpushbutton.h>

class ColorPalette : public QWidget {

public:

	ColorPalette();
	ColorPalette(QColor colors[16]);
	ColorPalette(QColor colors[16], QColor selected[2]);

	QColor getColor(u32 i);
	bool setColor(u32 i, QColor what);

	void mousePressEvent(QMouseEvent *me) override;

	QColor getLeft();
	QColor getRight();

	void setLeft(QColor what);
	void setRight(QColor what);

private:

	QColor colors[16];
	QPushButton *buttons[16];

	QColor selected[2];
	QPushButton *selectedButtons[2];

};