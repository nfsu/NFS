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
	shader.setUniformValue("primaryHighlight", primaryHighlight);
	shader.setUniformValue("secondaryHighlight", secondaryHighlight);
	shader.setUniformValue("rowHighlight", rowHighlight);
	shader.setUniformValue("primary", getPrimary());
	shader.setUniformValue("secondary", getSecondary());
	shader.setUniformValue("selectedRow", use4bit ? selectedRow : -1);

	quad.bind();

	int pos = shader.attributeLocation("pos");
	shader.enableAttributeArray(pos);
	shader.setAttributeBuffer(pos, GL_FLOAT, 0, 2);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (paletteTexture)
		paletteTexture->release(0);

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
		"uniform int primaryHighlight;"
		"uniform int secondaryHighlight;"
		"uniform int rowHighlight;"

		"uniform int primary;"
		"uniform int secondary;"
		"uniform int selectedRow;"

		"out vec4 color;"

		"void main() {"

			//Pad to 16x16 (even for 16x1 images)
			"ivec2 pos = ivec2(uv * 16);"
			"int coord1 = pos.x + pos.y * height;"
			"vec2 coord = vec2(pos) / vec2(16, height);"

			"vec2 delta = abs(uv * 16 - round(uv * 16));"

			"float minDelta = min(delta.x, delta.y);"

			"float overlay = 1 - min(minDelta, distToPix) / distToPix;"

			"uint value = texture(paletteTexture, coord).r;"

			"uint r = value & 0x1FU;"
			"uint g = (value & 0x3E0U) >> 5U;"
			"uint b = (value & 0x7C00U) >> 10U;"

			"vec3 sideColor = vec3(gridColor >> 16, (gridColor >> 8) & 0xFF, gridColor & 0xFF) / 255.f * 0.5f;"
			"vec3 primaryColor = vec3(primaryHighlight >> 16, (primaryHighlight >> 8) & 0xFF, primaryHighlight & 0xFF) / 255.f;"
			"vec3 secondaryColor = vec3(secondaryHighlight >> 16, (secondaryHighlight >> 8) & 0xFF, secondaryHighlight & 0xFF) / 255.f;"
			"vec3 rowColor = vec3(rowHighlight >> 16, (rowHighlight >> 8) & 0xFF, rowHighlight & 0xFF) / 255.f;"

			"sideColor = mix("
							"mix("
								"mix(sideColor, rowColor, float(pos.y == selectedRow) * 0.9f), "
								"secondaryColor, float(coord1 == secondary) * 0.9f"
							"),"
							"primaryColor, float(coord1 == primary) * 0.9f"
						");"

			//Sample from texture
			"color = vec4("
				"mix("
					"vec3(r, g, b) / 31.0f,"
					"sideColor,"
					"overlay"
				"), 1);"

		"}";

	//TODO: Selected index can look really weird

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
u8 PaletteRenderer::getPrimary() { return use4bit ? (primary & 0xF) + (selectedRow << 4) : primary; }
u8 PaletteRenderer::getSecondary() { return use4bit ? (secondary & 0xF) + (selectedRow << 4) : secondary; }
void PaletteRenderer::setShowGrid(bool b) { showGrid = b; repaint(); }
void PaletteRenderer::setEditable(bool b) { editable = b; repaint(); }
void PaletteRenderer::setGridSize(f32 perc) { gridSize = perc; repaint(); }
void PaletteRenderer::set4Bit(bool b) { use4bit = b; repaint(); }

void PaletteRenderer::setSelectedRow(u8 index) { 
	selectedRow = texture.getHeight() == 0 ? 0 : index % texture.getHeight(); 
	repaint(); 
}

void PaletteRenderer::setPrimary(u8 pos) {
	primary = use4bit ? pos & 0xF : pos;
}

void PaletteRenderer::setSecondary(u8 pos) {
	secondary = use4bit ? pos & 0xF : pos;
}

void PaletteRenderer::setGridColor(QColor color) {
	gridColor = (color.red() << 16) | (color.green() << 8) | color.blue();
	repaint();
}

void PaletteRenderer::setPrimaryHighlight(QColor color) {
	primaryHighlight = (color.red() << 16) | (color.green() << 8) | color.blue();
	repaint();
}

void PaletteRenderer::setSecondaryHighlight(QColor color) {
	secondaryHighlight = (color.red() << 16) | (color.green() << 8) | color.blue();
	repaint();
}

void PaletteRenderer::setRowHighlight(QColor color) {
	rowHighlight = (color.red() << 16) | (color.green() << 8) | color.blue();
	repaint();
}

void PaletteRenderer::set(QPoint p0, u32 color) {
	
	if (p0.x() < 0 || p0.y() < 0 || p0.x() >= texture.getWidth() || p0.y() >= texture.getHeight())
		return;

	texture.write(p0.x(), p0.y(), color);
	updateTexture();
}

u32 PaletteRenderer::get(QPoint p0) {

	if (p0.x() < 0 || p0.y() < 0 || p0.x() >= texture.getWidth() || p0.y() >= texture.getHeight())
		return 0;

	return texture.read(p0.x(), p0.y());

}

QPoint PaletteRenderer::globalToTexture(QPoint pos) {
	return QPoint((float)pos.x() / width() * texture.getWidth(), (float)pos.y() / height() * texture.getHeight());
}

//Texture

void PaletteRenderer::updateTexture() {
	destroyGTexture();
	setupGTexture();
}

void PaletteRenderer::reset() {
	texture = {};
	destroyGTexture();
	repaint();
}

void PaletteRenderer::destroyGTexture() {

	if (paletteTexture == nullptr) return;

	paletteTexture->destroy();
	delete paletteTexture;
	paletteTexture = nullptr;

}

void PaletteRenderer::setupGTexture() {

	if (texture.getWidth() == 0)
		return;

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

	if (e->button() == Qt::RightButton)
		setPrimary(pos);
	else if (e->button() == Qt::MiddleButton)
		setSecondary(pos);
	else if (editable && e->button() == Qt::LeftButton) {

		u32 color = get(tpos);

		QColorDialog dialog(QColor(color & 0xFF, (color & 0xFF00) >> 8, (color & 0xFF0000) >> 16));
		dialog.exec();

		QColor picked = dialog.selectedColor();

		if (picked.isValid())
			set(tpos, (picked.red() << 16) | (picked.green() << 8) | picked.blue());

	}

	repaint();

	//TODO: Allow painting in palette
	//TODO: Palette history

}

void PaletteRenderer::resizeEvent(QResizeEvent *e) {

	QOpenGLWidget::resizeEvent(e);

	QSize size = e->size();

	if (size.width() != size.height()) {
		i32 smallest = size.width() < size.height() ? size.width() : size.height();
		resize(smallest, smallest);
		updateGeometry();
	}

}
