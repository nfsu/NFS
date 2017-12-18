#include "ColorPalette.h"
#include <qgridlayout.h>
#include <qcolordialog.h>
#include <qevent.h>
#include <qpoint.h>

QColor defaultColors[16] = {

	QColor(255, 0, 0),
	QColor(0, 255, 0),

	QColor(0, 0, 255),
	QColor(255, 255, 0),

	QColor(255, 0, 255),
	QColor(0, 255, 255),

	QColor(255, 255, 255),
	QColor(0, 0, 0),


	QColor(127, 0, 0),
	QColor(0, 127, 0),

	QColor(0, 0, 127),
	QColor(127, 127, 0),

	QColor(127, 0, 127),
	QColor(0, 127, 127),

	QColor(127, 127, 127),
	QColor(63, 63, 63),
};

QColor defaultSelected[2] = {
	QColor(255, 255, 255),
	QColor(0, 0, 0)
};

ColorPalette::ColorPalette() : ColorPalette(defaultColors) {}
ColorPalette::ColorPalette(QColor colors[16]): ColorPalette(colors, defaultSelected) { }


void ColorPalette::mousePressEvent(QMouseEvent *me) {

	if (me->button() == Qt::RightButton)
		for (u32 i = 0; i < 16; ++i)
			if (buttons[i]->underMouse())
				setRight(getColor(i));

	if (me->button() == Qt::MiddleButton)
		for (u32 i = 0; i < 18; ++i)
			if (i < 16) {
				if(buttons[i]->underMouse())
					setColor(i, QColorDialog::getColor(getColor(i)));
			}
			else if (selectedButtons[i - 16]->underMouse()) {
				if(i == 16) setLeft(QColorDialog::getColor(getLeft()));
				else setRight(QColorDialog::getColor(getRight()));
			}
}

ColorPalette::ColorPalette(QColor colors[16], QColor selected[2]) {
	QGridLayout *colorLayout = new QGridLayout;
	setLayout(colorLayout);

	for (u32 j = 0; j < 9; ++j)
		for (u32 i = 0; i < 2; ++i) {

			if (j < 8) {
				QPushButton *c = new QPushButton;
				u32 k = i * 8 + j;
				c->setStyleSheet("background-color: #" + QString::number(colors[k].rgb(), 16));
				c->setMinimumSize(QSize(16, 16));
				c->setMaximumSize(QSize(64, 64));
				colorLayout->addWidget(c, j, i);
				buttons[k] = c;

				this->colors[k] = colors[k];

				connect(c, &QPushButton::pressed, this, [this, k]() { 
					this->setLeft(getColor(k));
				});

			}
			else {

				//TODO: Seperate this?

				QPushButton *c = new QPushButton;
				c->setStyleSheet("background-color: #" + QString::number(selected[i].rgb(), 16));
				c->setMinimumSize(QSize(16, 16));
				c->setMaximumSize(QSize(64, 64));
				colorLayout->addWidget(c, j, i);
				selectedButtons[i] = c;

				this->selected[i] = selected[i];
			}
		}

	setMaximumSize(QSize(32 + 16, 128 + 64));
}

QColor ColorPalette::getColor(u32 i) {
	if (i >= 16) return QColor(0, 0, 0);
	return colors[i];
}

bool ColorPalette::setColor(u32 i, QColor what) {
	if (i >= 16) return false;
	colors[i] = what;
	buttons[i]->setStyleSheet("background-color: #" + QString::number(colors[i].rgb(), 16));
	buttons[i]->update();
	return true;
}


QColor ColorPalette::getLeft() { return selected[0]; }
QColor ColorPalette::getRight() { return selected[1]; }

void ColorPalette::setLeft(QColor what) { 
	selected[0] = what;
	selectedButtons[0]->setStyleSheet("background-color: #" + QString::number(what.rgb(), 16));
	selectedButtons[0]->update();
}
void ColorPalette::setRight(QColor what) { 
	selected[1] = what;
	selectedButtons[1]->setStyleSheet("background-color: #" + QString::number(what.rgb(), 16));
	selectedButtons[1]->update();
}