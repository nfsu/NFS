#include "palette_renderer.hpp"

#pragma warning(push, 0)
	#include <QtGui/qevent.h>
	#include <QtGui/qopengltexture.h>
	#include <QtWidgets/qcolordialog.h>
#pragma warning(pop)

using namespace nfsu;

//Render

void PaletteRenderer::paintGL() {

	shader.bind();

	if(paletteTexture)
		paletteTexture->bind(0);

	shader.setUniformValue("paletteTexture", 0);
	shader.setUniformValue("height", qMax(i32(texture.getHeight()), 1));
	shader.setUniformValue("distToPix", showGrid ? gridSize : -gridSize);
	shader.setUniformValue("gridColor", gridColor);
	shader.setUniformValue("primary", getPrimary());

	glDrawArrays(GL_TRIANGLES, 0, 3);

	if (paletteTexture)
		paletteTexture->release(0);
}

//Initialize / destroy data

void PaletteRenderer::initializeGL() {

	//Setup shader

	//TODO: Clean this up

	const c8 *vertShader = R"(#version 330 core

		out vec2 uv;

		void main() {
			uv = vec2(ivec2(gl_VertexID << 1, gl_VertexID) & 2);
			gl_Position = vec4(uv * 2 - 1, 0, 1);
			uv.y = 1 - uv.y;
		}
	)";

	const c8 *fragShader = R"(#version 330 core

		in vec2 uv;

		uniform int height;
		uniform usampler2D paletteTexture;

		uniform float distToPix;
		uniform int gridColor;

		uniform int primary;

		out vec4 color;

		void main() {

			//Pad to 16x16 (even for 16x1 images)

			ivec2 pos = ivec2(uv * 16);
			int coord1 = pos.x + pos.y * 16;
			vec2 coord = vec2(pos) / vec2(16, height);

			vec2 delta = abs(uv * 16 - round(uv * 16));

			float minDelta = min(delta.x, delta.y);

			float dist = abs(distToPix);

			float overlay = 1 - floor(min(minDelta / 0.5, dist) / dist);

			uint value = texture(paletteTexture, coord).r;

			uvec3 rgb = (uvec3(value) >> uvec3(0u, 5u, 10u)) & 0x1Fu;
			uvec3 gridColorRgb = (uvec3(gridColor) >> uvec3(16u, 8u, 0u)) & 0xFFu;

			vec3 outColor = vec3(rgb) / 31.0f;

			vec3 sideColor = vec3(gridColorRgb) / 255.f * 0.5f;

			vec3 selectedColor = 1 - outColor;

			sideColor = mix(
				mix(
					sideColor, outColor, 0 - min(sign(distToPix), 0)
				), 
				selectedColor + (1 - outColor) * 1.5f, 
				float(coord1 == primary) * 0.9f
			);

			//Sample from texture
			color = vec4(
				mix(
					outColor,
					sideColor,
					overlay
				)
			, 1);
		})";

	if (!shader.addShaderFromSourceCode(QGLShader::Vertex, vertShader))
		EXCEPTION("Couldn't compile vertex shader");

	if (!shader.addShaderFromSourceCode(QGLShader::Fragment, fragShader))
		EXCEPTION("Couldn't compile fragment shader");

	if (!shader.link())
		EXCEPTION("Couldn't link shader");
}

PaletteRenderer::PaletteRenderer() : QOpenGLWidget() {
	setMouseTracking(true);
}

PaletteRenderer::~PaletteRenderer() {
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

//TODO: Move short setters/getters to header

QOpenGLTexture *PaletteRenderer::getGPUTexture() { return paletteTexture; }
u16 PaletteRenderer::getPrimary() { return primary; }
void PaletteRenderer::setShowGrid(bool b) { showGrid = b; repaint(); }
void PaletteRenderer::setEditable(bool b) { editable = b; repaint(); }
void PaletteRenderer::setGridSize(f32 perc) { gridSize = perc; repaint(); }

void PaletteRenderer::setGridColor(QColor color) {
	gridColor = (color.red() << 16) | (color.green() << 8) | color.blue();
	repaint();
}

void PaletteRenderer::set(QPoint p0, u32 color) {
	
	if (p0.x() < 0 || p0.y() < 0 || p0.x() >= int(texture.getWidth()) || p0.y() >= int(texture.getHeight()))
		return;

	texture.write(u16(p0.x()), u16(p0.y()), color);
	updateTexture();
}

u32 PaletteRenderer::get(QPoint p0) {

	if (p0.x() < 0 || p0.y() < 0 || p0.x() >= int(texture.getWidth()) || p0.y() >= int(texture.getHeight()))
		return 0;

	return texture.read(u16(p0.x()), u16(p0.y()));
}

QPoint PaletteRenderer::globalToTexture(QPoint pos) {
	return QPoint(
		int(f32(pos.x()) / width() * texture.getWidth()), 
		int(f32(pos.y()) / height() * texture.getHeight())
	);
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

	if (!paletteTexture) 
		return;

	paletteTexture->destroy();
	delete paletteTexture;
	paletteTexture = nullptr;
}

void PaletteRenderer::setupGTexture() {

	if (!texture.getWidth())
		return;

	paletteTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	paletteTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
	paletteTexture->setFormat(QOpenGLTexture::R16U);
	paletteTexture->setSize(texture.getWidth(), texture.getHeight());
	paletteTexture->allocateStorage();
	paletteTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt16, (const void*) texture.getPtr());

	repaint();
}

//Events

void PaletteRenderer::mousePressEvent(QMouseEvent *e) {

	QPoint tpos = globalToTexture(e->pos());

	if (editable && e->button() == Qt::LeftButton && texture.getWidth() != 0) {

		u32 color = get(tpos);

		QColorDialog dialog(QColor(color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF));
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

void PaletteRenderer::mouseMoveEvent(QMouseEvent *e) {
	QPoint tpos = globalToTexture(e->pos());
	primary = u8((tpos.y() << 4) | tpos.x());
	setFocus();
}

void PaletteRenderer::keyPressEvent(QKeyEvent *e) {

	if (e->key() == Qt::Key_X)
		setShowGrid(!showGrid);
}