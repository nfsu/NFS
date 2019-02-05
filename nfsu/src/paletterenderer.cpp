#include "paletterenderer.h"
#include <QtGui/qevent.h>
#include <QtWidgets/qcolordialog.h>
using namespace nfsu;

//Quad

const float quadData[] = {
	0,0, 0,1, 1,1,
	1,1, 1,0, 0,0
};

//Render

void PaletteRenderer::paintGL() {

	shader.bind();

	if(paletteTexture)
		paletteTexture->bind(0);

	shader.setUniformValue("paletteTexture", 0);
	shader.setUniformValue("height", (i32)texture.getHeight());
	shader.setUniformValue("distToPix", showGrid ? gridSize : 0);
	shader.setUniformValue("gridColor", gridColor);

	quad.bind();

	int pos = shader.attributeLocation("pos");
	shader.enableAttributeArray(pos);
	shader.setAttributeBuffer(pos, GL_FLOAT, 0, 2);

	glDrawArrays(GL_TRIANGLES, 0, 6);

}

//Initialize / destroy data

void PaletteRenderer::initializeGL() {

	//Setup vbo

	quad = QGLBuffer(QGLBuffer::VertexBuffer);
	quad.create();
	quad.bind();
	quad.allocate(quadData, sizeof(quadData));

	//Setup shader

	char const *vertShader = 

		"#version 330 core\r\n"

		"layout(location=0) in vec2 pos;"

		"out vec2 uv;"

		"void main() {"
			"uv = vec2(pos.x, 1 - pos.y);"
			"gl_Position = vec4(pos * 2 - 1, 0, 1);"
		"}";

	char const *fragShader =

		"#version 330 core\r\n"

		"in vec2 uv;"

		"uniform int height;"
		"uniform usampler2D paletteTexture;"

		"uniform float distToPix;"
		"uniform int gridColor;"

		"out vec4 color;"

		"void main() {"

			//Pad to 16x16 (even for 16x1 images)
			"ivec2 pos = ivec2(uv * 16);"
			"vec2 coord = vec2(pos) / vec2(16, height);"

			"vec2 delta = abs(uv * 16 - round(uv * 16));"

			"float minDelta = min(delta.x, delta.y);"

			"float overlay = 1 - min(minDelta, distToPix) / distToPix;"

			"uint value = texture(paletteTexture, coord).r;"

			"uint r = value & 0x1FU;"
			"uint g = (value & 0x3E0U) >> 5U;"
			"uint b = (value & 0x7C00U) >> 10U;"

			//Sample from texture
			"color = vec4("
				"mix("
					"vec3(r, g, b) / 31.0f,"
					"vec3(gridColor >> 16, (gridColor >> 8) & 0xFF, gridColor & 0xFF) / 255.f,"
					"overlay"
				"), 1);"

		"}";

	if (!shader.addShaderFromSourceCode(QGLShader::Vertex, vertShader))
		throw std::runtime_error("Couldn't compile vertex shader");

	if (!shader.addShaderFromSourceCode(QGLShader::Fragment, fragShader))
		throw std::runtime_error("Couldn't compile fragment shader");

	if (!shader.link())
		throw std::runtime_error("Couldn't link shader");


}

PaletteRenderer::~PaletteRenderer() {
	quad.destroy();
	shader.deleteLater();
}

//Getters / setters

void PaletteRenderer::setTexture(nfs::Texture2D tex) {
	texture = tex;
	updateTexture();
}

nfs::Texture2D PaletteRenderer::getTexture() {
	return texture;
}

QOpenGLTexture *PaletteRenderer::getGPUTexture() { return paletteTexture; }
QGLBuffer PaletteRenderer::getQuad() { return quad; }
u8 PaletteRenderer::getPrimary() { return primary; }
u8 PaletteRenderer::getSecondary() { return secondary; }
void PaletteRenderer::setShowGrid(bool b) { showGrid = b; repaint(); }
void PaletteRenderer::setEditable(bool b) { editable = b; repaint(); }
void PaletteRenderer::setGridSize(f32 perc) { gridSize = perc; repaint(); }

void PaletteRenderer::setGridColor(QColor color) {
	gridColor = (color.red() << 16) | (color.green() << 8) | color.blue();
	repaint();
}

void PaletteRenderer::set(QPoint p0, u32 color) {
	
	if (p0.x() < 0 || p0.y() < 0 || p0.x() >= texture.getWidth() || p0.y() >= texture.getHeight())
		return;

	texture.write(p0.x(), p0.y(), color);
	updateTexture();
}

QPoint PaletteRenderer::globalToTexture(QPoint pos) {
	return QPoint((float)pos.x() / width() * texture.getWidth(), (float)pos.y() / height() * texture.getHeight());
}

//Texture

void PaletteRenderer::updateTexture() {
	destroyGTexture();
	setupGTexture();
}

void PaletteRenderer::destroyGTexture() {

	if (paletteTexture == nullptr) return;

	paletteTexture->destroy();
	delete paletteTexture;
}

void PaletteRenderer::setupGTexture() {

	paletteTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	paletteTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
	paletteTexture->setFormat(QOpenGLTexture::R16U);
	paletteTexture->setSize(texture.getWidth(), texture.getHeight());
	paletteTexture->allocateStorage();
	paletteTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt16, texture.getPtr());

	repaint();

}

//Events

void PaletteRenderer::mousePressEvent(QMouseEvent *e) {

	QPoint tpos = globalToTexture(e->pos());
	u8 pos = (tpos.y() << 4) | tpos.x();

	//TODO: Show picked color
	//TODO: 4-bit mode (don't allow painting 8-bit)

	if (e->button() == Qt::LeftButton)
		primary = pos;
	else if (e->button() == Qt::RightButton)
		secondary = pos;
	else if (editable && Qt::MiddleButton) {

		QColorDialog dialog;
		dialog.exec();

		QColor picked = dialog.selectedColor();

		if (picked.isValid()) {
			set(tpos, (picked.red() << 16) | (picked.green() << 8) | picked.blue());
		}
	}

}