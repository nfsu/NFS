#include "tilerenderer.h"
#include "paletterenderer.h"
#include <QtGui/qevent.h>
using namespace nfsu;
using namespace nfs;

//Render

void TileRenderer::paintGL() {

	shader.bind();

	if (tiledTexture)
		tiledTexture->bind(0);

	if(paletteRenderer->getGPUTexture())
		paletteRenderer->getGPUTexture()->bind(1);

	if (magicTexture)
		magicTexture->bind(2);

	shader.setUniformValue("tiledTexture", 0);
	shader.setUniformValue("paletteTexture", 1);
	shader.setUniformValue("magicTexture", 2);
	shader.setUniformValue("width", (i32)texture.getWidth());
	shader.setUniformValue("height", (i32)texture.getHeight());
	shader.setUniformValue("tiled", (i32)texture.getTiles());
	shader.setUniformValue("size", (i32)texture.getDataSize());
	shader.setUniformValue("paletteY", (i32) yOffset);
	shader.setUniformValue("flags",
		(texture.getType() == TextureType::R4 ? 1 : 0) |
		(palette && paletteRenderer->getGPUTexture() != nullptr ? 2 : 0) |
		(magicTexture != nullptr ? 4 : 0)
	);

	paletteRenderer->getQuad().bind();

	int pos = shader.attributeLocation("pos");
	shader.enableAttributeArray(pos);
	shader.setAttributeBuffer(pos, GL_FLOAT, 0, 2);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (tiledTexture)
		tiledTexture->release(0);

	if (paletteRenderer->getGPUTexture())
		paletteRenderer->getGPUTexture()->release(1);

	if (magicTexture)
		magicTexture->release(2);

}

//Setup renderer

TileRenderer::TileRenderer(PaletteRenderer *palette): paletteRenderer(palette) {
	setScale(0);
}

void TileRenderer::initializeGL() {

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

		"uniform int width;"
		"uniform int height;"
		"uniform int size;"
		"uniform int tiled;"
		"uniform int flags;"
		"uniform int paletteY;"

		"uniform usampler2D tiledTexture;"
		"uniform usampler2D paletteTexture;"
		"uniform usampler2D magicTexture;"

		"out vec4 color;"

		"void main() {"

			//Convert from pixel to tiled pixel space

			"ivec2 px = ivec2(uv * vec2(width, height));"

			"int val = 0, mod2x4 = 0;"

			"if(tiled != 1) {"

				"ivec2 tile = px % tiled;"
				"ivec2 tiles = px / tiled;"
				"int pos = (tiles.x + tiles.y * width / tiled) * tiled * tiled + tile.y * tiled + tile.x;"

				//Convert from pixel index to buffer index

				"mod2x4 = (pos % 2) * 4;"
				"int texWidth = width;"

				"if((flags & 1) != 0) {"
					"pos /= 2;"
					"texWidth /= 2;"
				"}"

				"px = ivec2(pos % texWidth, pos / texWidth);"

			"} else {"

				"mod2x4 = (px.x % 2) * 4;"

				"if((flags & 1) != 0)"
					"px.x /= 2;"

			"}"

			"val = int(texelFetch(tiledTexture, px, 0).r);"

			"if((flags & 4) != 0)"
				"val ^= int(texelFetch(magicTexture, px, 0).r);"

			"if((flags & 1) != 0)"
				"val = (val & (0xF << mod2x4)) >> mod2x4;"

			//Convert from palette index to color

			"if((flags & 2) != 0) {"

				//Palette offset
				"if((flags & 1) != 0) val |= paletteY << 4;"

				//Get palette color
				"vec2 coord = vec2(val & 0xF, (val & 0xF0) >> 4) / vec2(16, 16);"
				"uint value = texture(paletteTexture, coord).r;"

				"uint r = value & 0x1FU;"
				"uint g = (value & 0x3E0U) >> 5U;"
				"uint b = (value & 0x7C00U) >> 10U;"

				//Sample from texture
				"color = vec4(r / 31.0f, g / 31.0f, b / 31.0f, 1);"

			"} else "
				"color = vec4((val & 0xF) / 15.f, ((val & 0xF0) >> 4) / 15.f, 0, 1);"
		"}";

	if(!shader.addShaderFromSourceCode(QGLShader::Vertex, vertShader))
		throw std::runtime_error("Couldn't compile vertex shader");

	if (!shader.addShaderFromSourceCode(QGLShader::Fragment, fragShader))
		throw std::runtime_error("Couldn't compile fragment shader");

	if (!shader.link())
		throw std::runtime_error("Couldn't link shader");

	//TODO: Some images still don't render well; detect resolution (u16_MAX for width & height)!

}

//Get texture

Texture2D TileRenderer::getTexture() {
	return texture;
}

//Clean up resources

TileRenderer::~TileRenderer() {
	shader.deleteLater();
	destroyGTexture();
}

//Texture functions

void TileRenderer::setTexture(Texture2D tex) {
	texture = tex;
	paletteRenderer->set4Bit(tex.getType() == TextureType::R4);
	setScale(scale);
	updateTexture();
}

void TileRenderer::destroyGTexture() {

	if (tiledTexture == nullptr) return;

	tiledTexture->destroy();
	delete tiledTexture;
	tiledTexture = nullptr;

	if (magicTexture == nullptr) return;

	magicTexture->destroy();
	delete magicTexture;
	magicTexture = nullptr;

}

void TileRenderer::setupGTexture() {

	if (texture.getWidth() == 0)
		return;

	tiledTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	tiledTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
	tiledTexture->setFormat(QOpenGLTexture::R8U);
	tiledTexture->setSize(texture.getDataSize() / texture.getHeight(), texture.getHeight());
	tiledTexture->allocateStorage();
	tiledTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt8, texture.getPtr());

	if (texture.useEncryption()) {
		magicTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
		magicTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
		magicTexture->setFormat(QOpenGLTexture::R8U);
		magicTexture->setSize(texture.getDataSize() / texture.getHeight(), texture.getHeight());
		magicTexture->allocateStorage();
		magicTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt8, texture.getMagicTexture());
	}

	repaint();

}

//Settings

void TileRenderer::usePalette(bool b) {
	palette = b;
	repaint();
}

void TileRenderer::setEditable(bool b) {
	editable = b;
	repaint();
}

void TileRenderer::setCursorSize(u32 scale) {
	cursorSize = scale;
	repaint();
}

void TileRenderer::setPaletteOffset(u8 j) {
	yOffset = j % 16;
	paletteRenderer->setSelectedRow(j);
	repaint();
}

void TileRenderer::setPaintTool(TilePaintTool t) {
	tool = t;
	repaint();
}

void TileRenderer::setScale(u32 s) {

	scale = s;

	if(texture.getWidth() == 0)
		setFixedSize(512, 512);		//TODO: Take maximum size
	else {

		if(s == 0)
			s = 512 / (texture.getHeight() > texture.getWidth() ? texture.getHeight() : texture.getWidth());
		
		if (s == 0)
			s = 1;

		setFixedSize(texture.getWidth() * s, texture.getHeight() * s);
	}

}

u32 TileRenderer::getSelectedPalette() {
	return isLeft ? paletteRenderer->getPrimary() : paletteRenderer->getSecondary();
}

//Drawing on tilemap

QPoint TileRenderer::globalToTexture(QPoint pos) {
	return QPoint((float)pos.x() / width() * texture.getWidth(), (float)pos.y() / height() * texture.getHeight());
}

void TileRenderer::mousePressEvent(QMouseEvent *e) {

	if (isMouseDown || !(e->button() == Qt::LeftButton || e->button() == Qt::RightButton))
		return;

	isLeft = e->button() == Qt::LeftButton;

	isMouseDown = true;
	prev = globalToTexture(e->pos());

	if (tool == TilePaintTool::FILL){
		fill(prev);
	} else
		mouseMoveEvent(e);

}

void TileRenderer::mouseReleaseEvent(QMouseEvent *e) {

	if (!isMouseDown || isLeft != (e->button() == Qt::LeftButton))
		return;

	if (tool == TilePaintTool::LINE) {

		//TODO: Draw overlay so you can see what line you're drawing
		//TODO: Show tool when you hover
		//TODO: Allow cancel

		drawLine(prev, globalToTexture(e->pos()));
		updateTexture();

	} else if(tool == TilePaintTool::SQUARE){

		//TODO: Draw overlay so you can see what square you're drawing
		//TODO: Allow cancel
		
		drawSquare(prev, globalToTexture(e->pos()));
		updateTexture();

	}

	//TODO: Color pipet

	isMouseDown = false;

}

void TileRenderer::drawPoint(QPoint point, u32 size) {

	if (!editable || point.x() < 0 || point.y() < 0 || point.x() >= width() || point.y() >= height())
		return;

	if (size == 0)
		size = cursorSize;

	i32 x = point.x(), y = point.y();

	i32 sx = x - size / 2;
	i32 sy = y - size / 2;
	i32 ex = x + size / 2;
	i32 ey = y + size / 2;

	if (sx == ex) {
		++ex;
		++ey;
	}

	for (i32 i = sx; i < ex; ++i)
		for (i32 j = sy; j < ey; ++j) {

			if (i < 0 || j < 0 || i >= texture.getWidth() || j >= texture.getHeight())
				continue;

			texture.store(i, j, getSelectedPalette());
		}

}

void TileRenderer::fill(i32 x, i32 y, u32 mask) {

	if (!editable || x < 0 || y < 0 || x >= texture.getWidth() || y >= texture.getHeight())
		return;

	u32 val = texture.fetch(x, y);

	if (val != mask)
		return;

	texture.store(x, y, getSelectedPalette());

	fill(x - 1, y, mask);
	fill(x + 1, y, mask);
	fill(x, y - 1, mask);
	fill(x, y + 1, mask);

}

void TileRenderer::fill(QPoint p0) {

	if (!editable || p0.x() < 0 || p0.y() < 0 || p0.x() >= texture.getWidth() || p0.y() >= texture.getHeight())
		return;

	u32 mask = texture.fetch(p0.x(), p0.y());

	if(mask != getSelectedPalette())
		fill(p0.x(), p0.y(), mask);

	updateTexture();

}

void TileRenderer::drawLine(QPoint p0, QPoint p1, u32 size) {

	if (p0 == p1) {
		drawPoint(p0, size);
		return;
	}

	QPoint dif = p1 - p0;

	i32 length = dif.x() * dif.x() + dif.y() * dif.y();

	float dx = float(dif.x()) / length, dy = float(dif.y()) / length;

	for (i32 i = 0; i < length; ++i)
		drawPoint(QPoint(i32(p0.x() + dx * i), i32(p0.y() + dy * i)), size);

}

void TileRenderer::drawSquare(QPoint p0, QPoint p1) {

	i32 x0 = p0.x(), y0 = p0.y(), x1 = p1.x(), y1 = p1.y();

	i32 mix = x0 < x1 ? x0 : x1;
	i32 miy = y0 < y1 ? y0 : y1;
	i32 max = x0 > x1 ? x0 : x1;
	i32 may = y0 > y1 ? y0 : y1;

	if (mix == max)
		max = mix + 1;

	if (miy == may)
		may = miy + 1;

	for(i32 x = mix; x < max; ++x)
		for (i32 y = miy; y < may; ++y) {

			if (x < 0 || y < 0 || x >= texture.getWidth() || y >= texture.getHeight())
				continue;

			texture.store(x, y, getSelectedPalette());

		}

}

void TileRenderer::mouseMoveEvent(QMouseEvent *e) {

	if (texture.getWidth() == 0 || !isMouseDown || !editable || tool != TilePaintTool::BRUSH)
		return;

	QPoint next = globalToTexture(e->pos());

	drawLine(prev, next);
	prev = next;

	updateTexture();

}

void TileRenderer::updateTexture() {
	destroyGTexture();
	setupGTexture();
}

void TileRenderer::reset() {
	texture = {};
	destroyGTexture();
	paletteRenderer->reset();
}