#include "NEditors.h"
#include <qsplitter.h>
#include <qboxlayout.h>
#include <qpushbutton.h>
#include "InfoTable.h"
#include <qheaderview.h>
#include <qcolordialog.h>
#include "ColorPalette.h"

NEditor *NEditors::add(QSplitter *parent, u32 mode) {

	QSplitter *content = new QSplitter(Qt::Vertical);

	QWidget *fileOptions = new QWidget;
	fileOptions->setContentsMargins(QMargins(0, 0, 0, 0));
	QHBoxLayout *hl = new QHBoxLayout;
	hl->setContentsMargins(QMargins(0, 5, 0, 5));
	fileOptions->setLayout(hl);
	hl->setAlignment(Qt::AlignHCenter);

	QPushButton *opt[4];
	hl->addWidget(opt[0] = new QPushButton("Save"));
	hl->addWidget(opt[1] = new QPushButton("Export"));
	hl->addWidget(opt[2] = new QPushButton("Load"));
	hl->addWidget(opt[3] = new QPushButton("Import"));

	for (QPushButton *op : opt) {
		op->setMinimumSize(QSize(64, 24));
		op->setMaximumSize(QSize(64, 24));
		op->setEnabled(false);
	}

	QSplitter *top = new QSplitter;

	NEditor *result = new NEditor(mode, buffers, textures, files, opt);
	result->setMinimumSize(QSize(256, 256));

	QWidget *right = new QWidget;
	QLayout *rightLayout = new QVBoxLayout;
	right->setLayout(rightLayout);

	if (mode == (u32)NEditorMode::PALETTE) {

		ColorPalette *color = new ColorPalette;
		rightLayout->addWidget(color);
	}

	top->addWidget(result);
	top->addWidget(right);
	top->setStretchFactor(0, 0);
	top->setStretchFactor(1, 1);

	content->addWidget(top);
	content->addWidget(fileOptions);
	content->setStretchFactor(0, 1);
	content->setStretchFactor(1, 0);

	parent->addWidget(content);

	return result;
}

NEditors::NEditors(){

	editors = std::vector<NEditor*>(4);

	QSplitter *right;
	{
		right = new QSplitter(Qt::Vertical);
		{
			QSplitter *rr;

			{
				rr = new QSplitter(Qt::Horizontal);

				editors[0] = add(rr, (u32)NEditorMode::PALETTE);
				editors[1] = add(rr, (u32)NEditorMode::TILEMAP);

				rr->setStretchFactor(0, 0);
				rr->setStretchFactor(1, 0);
			}

			QSplitter *rl;

			{
				rl = new QSplitter(Qt::Horizontal);

				editors[2] = add(rl, (u32)NEditorMode::MAP);
				editors[3] = add(rl, (u32)NEditorMode::PALETTE);

				rl->setStretchFactor(0, 0);
				rl->setStretchFactor(1, 0);
			}

			right->addWidget(rr);
			right->addWidget(rl);
		}

	}

	QVBoxLayout *layout;
	setLayout(layout = new QVBoxLayout);
	layout->addWidget(right);
}



void destroyTexture(GLuint &texture) {
	glDeleteTextures(1, &texture);
	texture = 0;
}


GLuint makeTexture(Texture2D tex) {
	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	bool is5bit = (tex.tt & TextureType::BGR5) != 0;
	bool is4bit = (tex.tt & TextureType::B4) != 0;

	GLenum extFormat = is5bit ? GL_RGBA : (tex.stride == 3 ? GL_RGB : GL_RED_INTEGER);
	GLenum intFormat = is5bit ? GL_RGB5 : (tex.stride == 1 ? GL_R8UI : (tex.stride == 2 ? GL_R16UI : (tex.stride == 3 ? GL_RGB8 : GL_R32UI)));
	GLenum type = is5bit ? GL_UNSIGNED_SHORT_1_5_5_5_REV : (tex.stride == 4 ? GL_UNSIGNED_INT : (tex.stride == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE));

	glTexImage2D(GL_TEXTURE_2D, 0, intFormat, is4bit ? tex.width / 2 : tex.width, tex.height, 0, extFormat, type, tex.data);

	glBindTexture(GL_TEXTURE_2D, 0);

	logGLErrors();

	return id;
}

void NEditors::setTexture(u32 id, Texture2D t2d, nfs::FileSystemObject *obj) {

	GLuint &buffer = buffers[id];
	Texture2D &tex = textures[id];

	files[id] = obj;

	if (buffer != 0) destroyTexture(buffer);
	buffer = makeTexture(tex = t2d);

	for (auto elem : editors) {
		elem->activate();
		elem->update();
	}
}

NEditors::~NEditors() {
	for (auto tex : buffers)
		if (tex.second != 0) destroyTexture(tex.second);
}