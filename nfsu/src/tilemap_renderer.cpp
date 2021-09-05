#include "tilemap_renderer.hpp"
#include "palette_renderer.hpp"

#pragma warning(push, 0)
	#include <QtGui/qevent.h>
	#include <QtCore/qtimer.h>
	#include <QtWidgets/qapplication.h>
	#include <QtGui/qopengltexture.h>
#pragma warning(pop)

using namespace nfsu;
using namespace nfs;

//Render

void TilemapRenderer::paintGL() {

	shader.bind();

	if (tileTexture)
		tileTexture->bind(0);

	if(paletteTexture)
		paletteTexture->bind(1);

	if (magicTexture)
		magicTexture->bind(2);

	if (tilemapTexture)
		tilemapTexture->bind(3);

	shader.setUniformValue("tileTexture", 0);
	shader.setUniformValue("paletteTexture", 1);
	shader.setUniformValue("magicTexture", 2);
	shader.setUniformValue("tilemapTexture", 3);
	shader.setUniformValue("width", i32(tilemap.getWidth()));
	shader.setUniformValue("height", i32(tilemap.getHeight()));
	shader.setUniformValue("tileWidth", i32(tiles.getWidth()));
	shader.setUniformValue("tileHeight", i32(tiles.getHeight()));
	shader.setUniformValue("size", i32(tilemap.getDataSize()));
	shader.setUniformValue("tiled", i32(tiles.getTiles()));
	shader.setUniformValue("offset", offset);
	shader.setUniformValue("scale", scale);
	shader.setUniformValue("gridColor", gridColor);
	shader.setUniformValue("distToPix", useGrid ? gridSize : -gridSize);

	shader.setUniformValue("flags",
		(tiles.getType() == TextureType::R4 ? 1 : 0) |
		(usePalette && paletteTexture != nullptr ? 2 : 0) |
		(magicTexture != nullptr ? 4 : 0)
	);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	if (tileTexture)
		tileTexture->release(0);

	if (paletteTexture)
		paletteTexture->release(1);

	if (magicTexture)
		magicTexture->release(2);

	if (tilemapTexture)
		tilemapTexture->release(3);
}

//Setup renderer

TilemapRenderer::TilemapRenderer() {
	updateScale();
	setMouseTracking(true);
}

void TilemapRenderer::initializeGL() {

	//Setup shader

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

		uniform int width;
		uniform int height;
		uniform int tileWidth;
		uniform int tileHeight;
		uniform int size;
		uniform int tiled;
		uniform int flags;
		uniform int gridColor;
		uniform float distToPix;

		uniform vec2 offset;
		uniform vec2 scale;

		uniform usampler2D tilemapTexture;
		uniform usampler2D tiledTexture;
		uniform usampler2D paletteTexture;
		uniform usampler2D magicTexture;

		out vec4 color;

		void outputWithOverlay(vec3 output, ivec2 tileOffset) {

			//Overlay

			vec2 delta = abs(fract(vec2(tileOffset) / vec2(8)));

			float minDelta = min(delta.x, delta.y);

			uvec3 gridColorRgb = (uvec3(gridColor) >> uvec3(16u, 8u, 0u)) & 0xFFu;
			vec3 sideColor = vec3(gridColorRgb) / 255.f * 0.5f;

			float overlay = 1 - floor(min(minDelta / 0.5, distToPix) / distToPix);

			if(distToPix > 0)
				output = mix(output, sideColor, overlay);

			color = vec4(output, 1);
		}

		void main() {

			//Convert from pixel to tilemap and tile space

			vec2 uvPos = uv * scale + offset;
			ivec2 size = ivec2(width, height);

			ivec2 tilemapPx = ivec2(uvPos * vec2(size));
			tilemapPx %= size;

			tilemapPx += size * ivec2(lessThan(tilemapPx, ivec2(0)));

			ivec2 realPx = ivec2(uvPos * vec2(size * tiled));

			ivec2 tilePx = realPx - tilemapPx * tiled;
			ivec2 tilePxx = tilePx;

			//TODO: Fix looping

			//Fetch tilemap

			int tileInfo = int(texelFetch(tilemapTexture, tilemapPx, 0).r);

			int tilePos = tileInfo & 0x3FF;
			int tileScl = (tileInfo & 0xC00) >> 10;
			int tilePlt = tileInfo >> 12;

			if((tileScl & 1) != 0)
				tilePx.x = 7 - tilePx.x;

			if((tileScl & 2) != 0)
				tilePx.y = 7 - tilePx.y;

			if(tileWidth <= 0) {
				outputWithOverlay(
					vec3(
						(tilePos & 0x1F) / float(0x1F), 
						((tilePos >> 5) & 0x1F) / float(0x1F), 
						(tileInfo >> 10) / float(0x3F)
					),
					tilePxx
				);
				return;
			}

			//Fetch tiles

			int val = 0, mod2x4 = 0;

			int pos = (tilePos << 6) | (tilePx.y << 3) | tilePx.x;
			
			//Convert from pixel index to buffer index
			
			mod2x4 = (pos & 1) * 4;
			int texWidth = tileWidth;
			
			if((flags & 1) != 0) {
				pos /= 2;
				texWidth /= 2;
			}
			
			ivec2 px = ivec2(pos % texWidth, pos / texWidth);
			ivec2 pixx = px;

			val = int(texelFetch(tiledTexture, px, 0).r);

			if((flags & 4) != 0)
				val ^= int(texelFetch(magicTexture, px, 0).r);

			if((flags & 1) != 0)
				val = (val & (0xF << mod2x4)) >> mod2x4;

			//Convert from palette index to color

			vec3 output;

			if((flags & 2) != 0) {

				//Palette offset

				val |= tilePlt << 4;

				//Get palette color

				vec2 coord = vec2((ivec2(val) >> ivec2(0, 4)) & 0xF) / vec2(16);
				uint value = texture(paletteTexture, coord).r;

				uvec3 rgb = (uvec3(value) >> uvec3(0u, 5u, 10u)) & 0x1Fu;

				//Sample from texture

				output = vec3(rgb) / 31.0f;
			}

			else if((flags & 1) == 0) 
				output = vec3(val / 255.f, 0, 0);

			else output = vec3(val / 15.f, 0, 0);

			outputWithOverlay(output, tilePxx);
		})";

	if(!shader.addShaderFromSourceCode(QGLShader::Vertex, vertShader))
		EXCEPTION("Couldn't compile vertex shader");

	if (!shader.addShaderFromSourceCode(QGLShader::Fragment, fragShader))
		EXCEPTION("Couldn't compile fragment shader");

	if (!shader.link())
		EXCEPTION("Couldn't link shader");

	//TODO: Smaller images are annoying to edit; maybe use a scrollbar if smallest < 128? Or allow resizing width?

}

//Clean up resources

TilemapRenderer::~TilemapRenderer() {
	shader.deleteLater();
	destroyGTexture();
}

//Texture functions

void TilemapRenderer::setTilemap(Texture2D tex) {

	tilemap = tex;

	updateScale();
	updateTexture();

	scale = QVector2D(1, 1);
	offset = QVector2D(0, 0);
}

void TilemapRenderer::setPalette(Texture2D tex) {
	palette = tex;
	updateTexture();
}

void TilemapRenderer::setTiles(Texture2D tex) {
	tiles = tex;
	updateTexture();
}

Texture2D TilemapRenderer::getPalette() {
	return palette;
}

void TilemapRenderer::destroyGTexture() {

	if (paletteTexture) {
		paletteTexture->destroy();
		delete paletteTexture;
		paletteTexture = nullptr;
	}

	if (tileTexture) {
		tileTexture->destroy();
		delete tileTexture;
		tileTexture = nullptr;
	}

	if (tilemapTexture) {
		tilemapTexture->destroy();
		delete tilemapTexture;
		tilemapTexture = nullptr;
	}

	if (magicTexture) {
		magicTexture->destroy();
		delete magicTexture;
		magicTexture = nullptr;
	}
}

void TilemapRenderer::setupGTexture() {

	repaint();

	if (palette.getWidth()) {

		paletteTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
		paletteTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
		paletteTexture->setFormat(QOpenGLTexture::R16U);
		paletteTexture->setSize(palette.getWidth(), palette.getHeight());
		paletteTexture->allocateStorage();
		paletteTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt16, (const void*) palette.getPtr());
	}

	if (tilemap.getWidth()) {

		tilemapTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
		tilemapTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
		tilemapTexture->setFormat(QOpenGLTexture::R16U);
		tilemapTexture->setSize(tilemap.getWidth(), tilemap.getHeight());
		tilemapTexture->allocateStorage();
		tilemapTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt16, (const void*) tilemap.getPtr());
	}

	if (!tiles.getWidth())
		return;

	tileTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	tileTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
	tileTexture->setFormat(QOpenGLTexture::R8U);
	tileTexture->setSize(int(tiles.getDataSize() / tiles.getHeight()), int(tiles.getHeight()));
	tileTexture->allocateStorage();
	tileTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt8, (const void*) tiles.getPtr());

	if (tiles.useEncryption()) {
		magicTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
		magicTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
		magicTexture->setFormat(QOpenGLTexture::R8U);
		magicTexture->setSize(int(tiles.getDataSize() / tiles.getHeight()), int(tiles.getHeight()));
		magicTexture->allocateStorage();
		magicTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt8, (const void*) tiles.getMagicTexture());
	}
}

//Settings

void TilemapRenderer::setUsePalette(bool b) {
	usePalette = b;
	repaint();
}

void TilemapRenderer::setUseGrid(bool b) {
	useGrid = b;
	repaint();
}

void TilemapRenderer::setEditable(bool b) {
	editable = b;
	repaint();
}

void TilemapRenderer::setGridColor(QColor color) {
	gridColor = (color.red() << 16) | (color.green() << 8) | color.blue();
	repaint();
}

void TilemapRenderer::setGridSize(f32 perc) { 
	gridSize = perc;
	repaint(); 
}

QPoint TilemapRenderer::globalToPixel(QPoint pos) {

	QVector2D uv(f32(pos.x()) / width(), f32(pos.y()) / height());
	uv *= scale;
	uv += offset;
	uv *= QVector2D(float(tilemap.getWidth()), float(tilemap.getHeight()));

	return QPoint(int(uv.x()), int(uv.y()));
}

QPoint TilemapRenderer::pixelToTexture(QPoint pos) {

	if (!tilemap.getWidth())
		return {};

	QVector2D size = QVector2D(float(tilemap.getWidth()), float(tilemap.getHeight()));

	QPoint px{ i32(pos.x()), i32(pos.y()) };

	return {
		px.x() - i32(floor(px.x() / size.x()) * size.x()),
		px.y() - i32(floor(px.y() / size.y()) * size.y())
	};
}

QPoint TilemapRenderer::globalToTexture(QPoint pos) {
	return pixelToTexture(globalToPixel(pos));
}

//Key events

void TilemapRenderer::keyPressEvent(QKeyEvent *e) {

	if (e->key() == Qt::Key::Key_Control) {
		ctrl = true;
		setCursor(QCursor(Qt::CursorShape::OpenHandCursor));
	}

	else if(e->key() == Qt::Key::Key_X)
		setUsePalette(!usePalette);

	else if(e->key() == Qt::Key::Key_Z)
		setUseGrid(!useGrid);

	else if(e->key() == Qt::Key_Shift)
		shift = true;

	else if (e->key() == Qt::Key_Alt) {

		alt = true;

		if(!ctrl)
			setCursor(QCursor(QPixmap("resources/color_picker.png")));
	}

	e->accept();

	//TODO: CTRL + Z vs CTRL + SHIFT + Z aka CTRL + Y
}

void TilemapRenderer::keyReleaseEvent(QKeyEvent *e) {

	if (e->key() == Qt::Key::Key_Control) {

		ctrl = false;
		isMouseDown = false;

		if(!alt)
			setCursor(QCursor(Qt::CursorShape::ArrowCursor));

		else setCursor(QCursor(QPixmap("resources/color_picker.png")));
	}
	
	else if (e->key() == Qt::Key_Shift)
		shift = false;

	else if (e->key() == Qt::Key_Alt) {

		alt = false;

		if(!ctrl)
			setCursor(QCursor(Qt::ArrowCursor));
	}

	e->accept();
}

//Mouse events

void TilemapRenderer::wheelEvent(QWheelEvent *e) {

	i32 dif = e->angleDelta().y();

	if (ctrl) {

		f32 d = f32(-dif) / 120 * 0.05f + 1;
		scale *= d;

	}
}

void TilemapRenderer::mousePressEvent(QMouseEvent *e) {

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
	prev = e->pos();

	if (ctrl) {

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

	if (alt)
		return;

	mouseMoveEvent(e);
}

void TilemapRenderer::mouseReleaseEvent(QMouseEvent *e) {

	if (!isMouseDown || isLeft != (e->button() == Qt::LeftButton))
		return;

	isMouseDown = false;

	if (ctrl)
		setCursor(QCursor(Qt::CursorShape::OpenHandCursor));

	else if(!alt)
		setCursor(QCursor(Qt::CursorShape::ArrowCursor));
}

//TODO: Tooltips
//TODO: Toolbar for palette and tiles

void TilemapRenderer::mouseMoveEvent(QMouseEvent *e) {

	//TODO: Set cursor icon for painting
	//TODO: Set painter icon for painting

	if (!isMouseDown) {
		setFocus();
		return;
	}

	if (!tilemap.getWidth() || !isMouseDown)
		return;

	QPoint next = e->pos();

	if (next.x() < 0 || next.y() < 0 || next.x() >= width() || next.y() >= height())
		return;

	if (ctrl) {
		QPoint dif = prev - next;
		offset += QVector2D(f32(dif.x()) / tilemap.getWidth(), f32(dif.y()) / tilemap.getHeight()) * scale;
		prev = next;
		return;
	}
}

void TilemapRenderer::updateTexture() {
	destroyGTexture();
	setupGTexture();
}

void TilemapRenderer::reset() {
	tilemap = {};
	tiles = {};
	paletteTexture = {};
	destroyGTexture();
}

void TilemapRenderer::updateScale() {

	if (tilemap.getWidth() == 0) {
		setMinimumSize(256, 256);
		return;
	}

	else setMinimumSize(tilemap.getWidth(), tilemap.getHeight());

	f32 scaleX = f32(width()) / tilemap.getWidth();
	f32 scaleY = f32(height()) / tilemap.getHeight();

	if (scaleX == scaleY)
		return;

	if (scaleX < scaleY)
		scaleY = scaleX;

	else scaleX = scaleY;

	i32 sizeX = i32(scaleX * tilemap.getWidth() * 8);
	i32 sizeY = i32(scaleY * tilemap.getHeight() * 8);

	resize(sizeX, sizeY);
	updateGeometry();
}

//TODO: Center tile renderer, tilemap renderer and palette render
//TODO: Center toolbar

void TilemapRenderer::resizeEvent(QResizeEvent *e) {

	QOpenGLWidget::resizeEvent(e);

	if (!tilemap.getWidth())
		return;

	QSize size = e->size();

	f32 scaleX = f32(size.width()) / tilemap.getWidth();
	f32 scaleY = f32(size.height()) / tilemap.getHeight();

	if (scaleX == scaleY)
		return;

	if (scaleX < scaleY)
		scaleY = scaleX;

	else scaleX = scaleY;

	i32 sizeX = i32(scaleX * tilemap.getWidth());
	i32 sizeY = i32(scaleY * tilemap.getHeight());

	resize(sizeX, sizeY);		//TODO: This can cause an infinite loop
	updateGeometry();
}