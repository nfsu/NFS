#include "NEditors.h"
#include <qsplitter.h>
#include <qboxlayout.h>

NEditors::NEditors(){

	editors = std::vector<NEditor*>(4);

	QSplitter *right;
	{
		right = new QSplitter(Qt::Vertical);
		{
			QSplitter *rr;

			{
				rr = new QSplitter(Qt::Horizontal);

				editors[0] = new NEditor(NEditorMode::PALETTE, buffers, textures);
				editors[0]->setMinimumSize(QSize(500, 400));

				editors[1] = new NEditor(NEditorMode::TILEMAP, buffers, textures);
				editors[1]->setMinimumSize(QSize(500, 400));

				rr->addWidget(editors[0]);
				rr->addWidget(editors[1]);
			}

			QSplitter *rl;

			{
				rl = new QSplitter(Qt::Horizontal);

				editors[2] = new NEditor(NEditorMode::MAP, buffers, textures);
				editors[2]->setMinimumSize(QSize(500, 400));

				editors[3] = new NEditor(NEditorMode::PALETTE, buffers, textures);
				editors[3]->setMinimumSize(QSize(500, 400));

				rl->addWidget(editors[2]);
				rl->addWidget(editors[3]);
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

void NEditors::setTexture(u32 id, Texture2D t2d) {

	GLuint &buffer = buffers[id];
	Texture2D &tex = textures[id];

	if (buffer != 0) destroyTexture(buffer);
	buffer = makeTexture(tex = t2d);

	for(auto elem : editors)
		elem->update();
}

NEditors::~NEditors() {
	for (auto tex : buffers)
		if (tex.second != 0) destroyTexture(tex.second);
}