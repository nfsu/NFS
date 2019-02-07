#include "tilerenderer.h"
#include "paletterenderer.h"
#include <QtGui/qevent.h>
#include <QtCore/qtimer.h>
using namespace nfsu;
using namespace nfs;

//Render

void TileRenderer::paintGL() {

	if (!paletteRenderer->getQuad().isCreated())
		return;

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
	shader.setUniformValue("offset", offset);
	shader.setUniformValue("scale", scale);
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
	updateScale();
	setMouseTracking(true);
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

		"uniform vec2 offset;"
		"uniform vec2 scale;"

		"uniform usampler2D tiledTexture;"
		"uniform usampler2D paletteTexture;"
		"uniform usampler2D magicTexture;"

		"out vec4 color;"

		"void main() {"

			//Convert from pixel to tiled pixel space

			"vec2 pos = uv * scale + offset;"

			"ivec2 px = ivec2(pos * vec2(width, height)) % ivec2(width, height);"

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

			"vec3 output;"

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
				"output = vec3(r / 31.0f, g / 31.0f, b / 31.0f);"

			"} else "
				"output = vec3((val & 0xF) / 15.f, ((val & 0xF0) >> 4) / 15.f, 0);"

			"color = vec4("
						"mix(output, vec3(0), "
							"float(px.x < 0 || px.y < 0 || px.x >= width || px.y >= height))"
					", 1);"
		"}";

	if(!shader.addShaderFromSourceCode(QGLShader::Vertex, vertShader))
		throw std::runtime_error("Couldn't compile vertex shader");

	if (!shader.addShaderFromSourceCode(QGLShader::Fragment, fragShader))
		throw std::runtime_error("Couldn't compile fragment shader");

	if (!shader.link())
		throw std::runtime_error("Couldn't link shader");

	//TODO: Some images still don't render well; detect resolution (u16_MAX for width & height)!

	//TODO: Animate with specified palette offset & length

	//TODO: Smaller images are annoying to edit; maybe use a scrollbar if smallest < 128? Or allow resizing width?

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
	updateScale();
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

u32 TileRenderer::getSelectedPalette() {
	return isLeft ? paletteRenderer->getPrimary() : paletteRenderer->getSecondary();
}

QPoint TileRenderer::globalToTexture(QPoint pos) {

	QVector2D uv((float)pos.x() / width(), (float)pos.y() / height());
	uv *= scale;
	uv += offset;

	if (texture.getWidth() == 0)
		return {};

	return {
		u32(uv.x() * texture.getWidth()) % texture.getWidth(), 
		u32(uv.y() * texture.getHeight()) % texture.getHeight() 
	};
}

//Key events

void TileRenderer::keyPressEvent(QKeyEvent *e) {

	if (e->key() == Qt::Key::Key_Control) {
		specialKey = 1;
		setCursor(QCursor(Qt::CursorShape::OpenHandCursor));
	} else if(e->key() == Qt::Key::Key_X)
		usePalette(!palette);

	//TODO: CTRL + Z vs CTRL + SHIFT + Z aka CTRL + Y

}

void TileRenderer::keyReleaseEvent(QKeyEvent *e) {

	if (e->key() == Qt::Key::Key_Control) {
		specialKey = 0;
		isMouseDown = false;
		setCursor(QCursor(Qt::CursorShape::ArrowCursor));
	}

}

//Mouse events

void TileRenderer::wheelEvent(QWheelEvent *e) {

	//TODO: Shift = cursor size

	i32 dif = e->angleDelta().y();

	if (specialKey == 1) {

		float d = float(dif) / 120 * 0.05f + 1;
		scale *= d;

	} else if(texture.getType() == TextureType::R4) {

		if (dif < 0)
			setPaletteOffset((yOffset + 1) & 0xF);
		else
			setPaletteOffset((yOffset - 1) & 0xF);

	}

}

void TileRenderer::mousePressEvent(QMouseEvent *e) {

	if (!(e->button() == Qt::LeftButton || e->button() == Qt::RightButton))
		return;

	if (isMouseDown) {
		isMouseDown = false;
		setCursor(QCursor(Qt::CursorShape::ArrowCursor));
		return;
	}

	isLeft = e->button() == Qt::LeftButton;
	
	//TODO: Previous, next tool

	isMouseDown = true;
	prev = globalToTexture(e->pos());

	if (specialKey == 1) {

		setCursor(QCursor(Qt::CursorShape::ClosedHandCursor));

		if (!isLeft) {
			offset = QVector2D(0, 0);
			scale = QVector2D(1, 1);
			isMouseDown = false;
			setCursor(QCursor(Qt::CursorShape::ArrowCursor));
			return;
		}

		mouseMoveEvent(e);
		return;
	}

	if (tool == TilePaintTool::FILL){
		fill(prev);
	} else if(tool == TilePaintTool::EYEDROPPER){

		if (isLeft)
			paletteRenderer->setPrimary(get(prev));
		else
			paletteRenderer->setSecondary(get(prev));

	} else
		mouseMoveEvent(e);

}

void TileRenderer::mouseReleaseEvent(QMouseEvent *e) {

	if (!isMouseDown || isLeft != (e->button() == Qt::LeftButton))
		return;

	if (specialKey == 1)
		setCursor(QCursor(Qt::CursorShape::OpenHandCursor));
	else
		setCursor(QCursor(Qt::CursorShape::ArrowCursor));

	//TODO: Draw overlay so you can see what line/square you're drawing

	if (tool == TilePaintTool::LINE) {
		drawLine(prev, globalToTexture(e->pos()));
		updateTexture();
	} else if(tool == TilePaintTool::SQUARE){
		drawSquare(prev, globalToTexture(e->pos()));
		updateTexture();
	}

	//TODO: Move tool
	//TODO: Zoom tool
	//TODO: Select tool

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

//Drawing

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

u32 TileRenderer::get(QPoint p0) {

	if (p0.x() < 0 || p0.y() < 0 || p0.x() >= texture.getWidth() || p0.y() >= texture.getHeight())
		return 0;

	return texture.read(p0.x(), p0.y());
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

	//TODO: Set cursor icon

	if (!isMouseDown) {
		setFocus();
		return;
	}

	if (texture.getWidth() == 0 || !isMouseDown)
		return;

	//TODO: When you offset to make it loop, it thinks you move from right side of screen to left side
	//While in reality it should take the shortest route

	//TODO: Flickering!

	QPoint next = globalToTexture(e->pos());

	if (specialKey == 1) {
		QPoint dif = prev - next;
		offset += QVector2D((float) dif.x() / texture.getWidth(), (float) dif.y() / texture.getHeight());
		prev = next;
		return;
	}

	if (!editable || tool != TilePaintTool::BRUSH)
		return;

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

void TileRenderer::updateScale() {

	if (texture.getWidth() == 0)
		setMinimumSize(256, 256);
	else
		setMinimumSize(texture.getWidth(), texture.getHeight());

	float scaleX = (float) width() / texture.getWidth();
	float scaleY = (float) height() / texture.getHeight();

	if (scaleX == scaleY)
		return;

	if (scaleX < scaleY)
		scaleY = scaleX;
	else
		scaleX = scaleY;

	i32 sizeX = i32(scaleX * texture.getWidth());
	i32 sizeY = i32(scaleY * texture.getHeight());

	resize(sizeX, sizeY);
	updateGeometry();

}

void TileRenderer::resizeEvent(QResizeEvent *e) {

	QOpenGLWidget::resizeEvent(e);

	if (texture.getWidth() == 0)
		return;

	QSize size = e->size();

	float scaleX = (float)size.width() / texture.getWidth();
	float scaleY = (float)size.height() / texture.getHeight();

	if (scaleX == scaleY)
		return;

	if (scaleX < scaleY)
		scaleY = scaleX;
	else
		scaleX = scaleY;

	i32 sizeX = i32(scaleX * texture.getWidth());
	i32 sizeY = i32(scaleY * texture.getHeight());

	resize(sizeX, sizeY);
	updateGeometry();

}