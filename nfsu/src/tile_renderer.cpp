#include "tile_renderer.hpp"
#include "palette_renderer.hpp"

#pragma warning(push, 0)
	#include <QtGui/qevent.h>
	#include <QtGui/qopenglfunctions_3_3_core.h>
	#include <QtCore/qtimer.h>
	#include <QtWidgets/qapplication.h>
	#include <QtGui/qopengltexture.h>
#pragma warning(pop)

using namespace nfsu;
using namespace nfs;

//Render

void TileRenderer::paintGL() {

	shader.bind();

	if (tiledTexture)
		tiledTexture->bind(0);

	if(paletteTexture)
		paletteTexture->bind(1);

	if (magicTexture)
		magicTexture->bind(2);

	shader.setUniformValue("tiledTexture", 0);
	shader.setUniformValue("paletteTexture", 1);
	shader.setUniformValue("magicTexture", 2);
	shader.setUniformValue("width", i32(texture.getWidth()));
	shader.setUniformValue("height", i32(texture.getHeight()));
	shader.setUniformValue("tiled", i32(texture.getTiles()));
	shader.setUniformValue("size", i32(texture.getDataSize()));
	shader.setUniformValue("paletteY", i32(yOffset));
	shader.setUniformValue("offset", offset);
	shader.setUniformValue("scale", scale);
	shader.setUniformValue("gridColor", gridColor);
	shader.setUniformValue("distToPix", useGrid ? gridSize : -gridSize);

	shader.setUniformValue("flags",
		(texture.getType() == TextureType::R4 ? 1 : 0) |
		(usePalette && paletteTexture != nullptr ? 2 : 0) |
		(magicTexture != nullptr ? 4 : 0)
	);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	if (tiledTexture)
		tiledTexture->release(0);

	if (paletteTexture)
		paletteTexture->release(1);

	if (magicTexture)
		magicTexture->release(2);
}

//Setup renderer

TileRenderer::TileRenderer() {
	updateScale();
	setMouseTracking(true);
}

void TileRenderer::initializeGL() {

	auto *ctx = context();

	if (!ctx->versionFunctions<QOpenGLFunctions_3_3_Core>())
		EXCEPTION("Invalid GL version");

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
		uniform int size;
		uniform int tiled;
		uniform int flags;
		uniform int paletteY;
		uniform int gridColor;
		uniform float distToPix;

		uniform vec2 offset;
		uniform vec2 scale;

		uniform usampler2D tiledTexture;
		uniform usampler2D paletteTexture;
		uniform usampler2D magicTexture;

		out vec4 color;

		void main() {

			//Convert from pixel to tiled pixel space

			vec2 pos = uv * scale + offset;

			ivec2 px = ivec2(pos * vec2(width, height));
			ivec2 pixx = px;
			ivec2 size = ivec2(width, height);

			px = px - ivec2(floor(vec2(px) / size) * size);

			int val = 0, mod2x4 = 0;

			if(tiled == 8) {

				ivec2 tile = px & 7;
				ivec2 tiles = px >> 3;
				int pos = ((tiles.x + tiles.y * (width >> 3)) << 6) + (tile.y << 3) + tile.x;

				//Convert from pixel index to buffer index

				mod2x4 = (pos % 2) * 4;
				int texWidth = width;

				if((flags & 1) != 0) {
					pos /= 2;
					texWidth /= 2;
				}

				px = ivec2(pos % texWidth, pos / texWidth);

			} else {

				mod2x4 = (px.x % 2) * 4;

				if((flags & 1) != 0)
					px.x /= 2;
			}

			val = int(texelFetch(tiledTexture, px, 0).r);

			if((flags & 4) != 0)
				val ^= int(texelFetch(magicTexture, px, 0).r);

			if((flags & 1) != 0)
				val = (val & (0xF << mod2x4)) >> mod2x4;

			//Convert from palette index to color

			vec3 output;

			if((flags & 2) != 0) {

				//Palette offset

				if((flags & 1) != 0) 
					val |= paletteY << 4;

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

			//Overlay

			vec2 delta = abs(fract(pixx / vec2(8)));

			float minDelta = min(delta.x, delta.y);

			uvec3 gridColorRgb = (uvec3(gridColor) >> uvec3(16u, 8u, 0u)) & 0xFFu;
			vec3 sideColor = vec3(gridColorRgb) / 255.f * 0.5f;

			float overlay = 1 - floor(min(minDelta / 0.5, distToPix) / distToPix);

			if(distToPix > 0)
				output = mix(output, sideColor, overlay);

			color = vec4(
				mix(
					output, vec3(0), 
					float(
						any(lessThan(px, ivec2(0))) || 
						any(greaterThanEqual(px, ivec2(width, height)))
					)
				)
				, 1
			);
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

TileRenderer::~TileRenderer() {
	shader.deleteLater();
	destroyGTexture();
}

//Texture functions

void TileRenderer::setTexture(Texture2D tex) {

	texture = tex;
	//paletteRenderer->set4Bit(tex.getType() == TextureType::R4);

	setPaletteOffset(0);

	updateScale();
	updateTexture();

	scale = QVector2D(1, 1);
	offset = QVector2D(0, 0);
}

void TileRenderer::setPalette(nfs::Texture2D tex) {
	palette = tex;
	updateTexture();
}

nfs::Texture2D TileRenderer::getPalette(){
	return palette;
}

void TileRenderer::destroyGTexture() {

	if (paletteTexture) {
		paletteTexture->destroy();
		delete paletteTexture;
		paletteTexture = nullptr;
	}

	if (!tiledTexture) 
		return;

	tiledTexture->destroy();
	delete tiledTexture;
	tiledTexture = nullptr;

	if (!magicTexture) 
		return;

	magicTexture->destroy();
	delete magicTexture;
	magicTexture = nullptr;
}

void TileRenderer::setupGTexture() {

	repaint();

	if (palette.getWidth()) {

		paletteTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
		paletteTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
		paletteTexture->setFormat(QOpenGLTexture::R16U);
		paletteTexture->setSize(palette.getWidth(), palette.getHeight());
		paletteTexture->allocateStorage();
		paletteTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt16, (const void*) palette.getPtr());
	}

	if (!texture.getWidth())
		return;

	tiledTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	tiledTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
	tiledTexture->setFormat(QOpenGLTexture::R8U);
	tiledTexture->setSize(int(texture.getDataSize() / texture.getHeight()), int(texture.getHeight()));
	tiledTexture->allocateStorage();
	tiledTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt8, (const void*) texture.getPtr());

	if (texture.useEncryption()) {
		magicTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
		magicTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
		magicTexture->setFormat(QOpenGLTexture::R8U);
		magicTexture->setSize(int(texture.getDataSize() / texture.getHeight()), int(texture.getHeight()));
		magicTexture->allocateStorage();
		magicTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt8, (const void*) texture.getMagicTexture());
	}
}

//Settings

void TileRenderer::setUsePalette(bool b) {
	usePalette = b;
	repaint();
}

void TileRenderer::setUseGrid(bool b) {
	useGrid = b;
	repaint();
}

void TileRenderer::setEditable(bool b) {
	editable = b;
	repaint();
}

void TileRenderer::setCursorSize(u32 cursorSiz) {
	cursorSize = cursorSiz;
	repaint();
}

void TileRenderer::setGridColor(QColor color) {
	gridColor = (color.red() << 16) | (color.green() << 8) | color.blue();
	repaint();
}

void TileRenderer::setGridSize(f32 perc) { gridSize = perc; repaint(); }

void TileRenderer::setPaletteOffset(u8 j) {
	yOffset = j % 16;
	repaint();
}

void TileRenderer::setPaintTool(TilePaintTool t) {
	tool = t;
	repaint();
}

u32 TileRenderer::getSelectedPalette() {
	return 0 /* TODO: */;
}

QPoint TileRenderer::globalToPixel(QPoint pos) {

	QVector2D uv(f32(pos.x()) / width(), f32(pos.y()) / height());
	uv *= scale;
	uv += offset;
	uv *= QVector2D(float(texture.getWidth()), float(texture.getHeight()));

	return QPoint(int(uv.x()), int(uv.y()));
}

QPoint TileRenderer::pixelToTexture(QPoint pos) {

	if (!texture.getWidth())
		return {};

	QVector2D size = QVector2D(float(texture.getWidth()), float(texture.getHeight()));

	QPoint px{ i32(pos.x()), i32(pos.y()) };

	return {
		px.x() - i32(floor(px.x() / size.x()) * size.x()),
		px.y() - i32(floor(px.y() / size.y()) * size.y())
	};
}

QPoint TileRenderer::globalToTexture(QPoint pos) {
	return pixelToTexture(globalToPixel(pos));
}

//Key events

void TileRenderer::keyPressEvent(QKeyEvent *e) {

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

void TileRenderer::keyReleaseEvent(QKeyEvent *e) {

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

void TileRenderer::wheelEvent(QWheelEvent *e) {

	i32 dif = e->angleDelta().y();

	if (ctrl) {

		f32 d = f32(-dif) / 120 * 0.05f + 1;
		scale *= d;

	} else if(shift){
	
		if (dif > 0)
			setCursorSize(cursorSize + 1);

		else if (cursorSize > 1)
			setCursorSize(cursorSize - 1);

	} else if(texture.getType() == TextureType::R4) {

		if (dif < 0)
			setPaletteOffset((yOffset + 1) & 0xF);

		else setPaletteOffset((yOffset - 1) & 0xF);
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

	if (alt) {

		/*if (isLeft)
			paletteRenderer->setPrimary(get(prev));
		else
			paletteRenderer->setSecondary(get(prev));*/

		return;
	}

	if (tool == TilePaintTool::FILL)
		fill(globalToTexture(prev));

	else mouseMoveEvent(e);
}

void TileRenderer::mouseReleaseEvent(QMouseEvent *e) {

	if (!isMouseDown || isLeft != (e->button() == Qt::LeftButton))
		return;

	isMouseDown = false;

	if (ctrl)
		setCursor(QCursor(Qt::CursorShape::OpenHandCursor));

	else if(!alt)
		setCursor(QCursor(Qt::CursorShape::ArrowCursor));

	//TODO: Draw overlay so you can see what line/square you're drawing

	QPoint next = e->pos();

	next = QPoint(qBound(0, next.x(), width() - 1), qBound(0, next.y(), height() - 1));

	if (ctrl)
		return;

	if (tool == TilePaintTool::LINE) {
		drawLine(globalToPixel(prev), globalToPixel(next));
		updateTexture();
	}
	
	else if(tool == TilePaintTool::SQUARE){
		drawSquare(globalToPixel(prev), globalToPixel(next));
		updateTexture();
	}

	//TODO: Select tool
}

void TileRenderer::drawPoint(QPoint point) {
	drawSquare(
		point - QPoint(cursorSize / 2, cursorSize / 2), 
		point + QPoint(cursorSize / 2, cursorSize / 2)
	);
}

//Drawing

void TileRenderer::fill(QPoint p0) {

	if (!editable || p0.x() < 0 || p0.y() < 0 || p0.x() >= int(texture.getWidth()) || p0.y() >= int(texture.getHeight()))
		return;

	u32 mask = texture.fetch(u16(p0.x()), u16(p0.y()));
	u32 targ = getSelectedPalette();

	if (mask != targ) {

		u32 w = texture.getWidth(), h = texture.getHeight(), j = w * h, i = 0;
		List<bool> bits(j);

		List<u32> marked, marked2;

		marked.reserve(j);
		marked.push_back(p0.y() * w + p0.x());

		bits[p0.y() * w + p0.x()] = false;

		marked2.reserve(j);

		for (; i < j; ++i) {

			u32 val = texture.fetch(u16(i % w), u16(i / w));
			bits[i] = val == mask;
		}

		do {

			for (i = 0, j = u32(marked.size()); i < j; ++i) {

				u32 xy = marked[i], x = xy % w, y = xy / w;

				texture.store(u16(x), u16(y), targ);

				if (x && bits[xy - 1]) {
					marked2.push_back(xy - 1);
					bits[xy - 1] = false;
				}

				if (x != w - 1 && bits[xy + 1]) {
					marked2.push_back(xy + 1);
					bits[xy + 1] = false;
				}

				if (y && bits[xy - w]) {
					marked2.push_back(xy - w);
					bits[xy - w] = false;
				}

				if (y != h - 1 && bits[xy + w]) {
					marked2.push_back(xy + w);
					bits[xy + w] = false;
				}
			}

			marked = marked2;
			marked2.clear();

		} while (marked.size());
	}

	updateTexture();
}

u32 TileRenderer::get(QPoint p0) {

	p0 = globalToTexture(p0);

	if (p0.x() < 0 || p0.y() < 0 || p0.x() >= int(texture.getWidth()) || p0.y() >= int(texture.getHeight()))
		return 0;

	return texture.read(u16(p0.x()), u16(p0.y()));
}

void TileRenderer::drawLine(QPoint p0, QPoint p1) {

	if (p0 == p1) {
		drawPoint(p0);
		return;
	}

	QPoint dif = p1 - p0;

	if (abs(dif.x()) > abs(dif.y())) {

		f32 yPerX = f32(dif.y()) / dif.x();
		i32 sign = dif.x() < 0 ? -1 : 1;

		for (i32 i = 0, l = abs(dif.x()); i < l; ++i) {
			i32 x = sign * i;
			i32 y = i32(yPerX * x);
			drawPoint(QPoint(p0.x() + x, p0.y() + y));
		}

		return;
	}

	f32 xPerY = f32(dif.x()) / dif.y();
	i32 sign = dif.y() < 0 ? -1 : 1;

	for (i32 j = 0, l = abs(dif.y()); j < l; ++j) {
		i32 y = sign * j;
		i32 x = i32(xPerY * y);
		drawPoint(QPoint(p0.x() + x, p0.y() + y));
	}
}

void TileRenderer::drawSquare(QPoint p0, QPoint p1) {

	i32 x0 = p0.x(), y0 = p0.y(), x1 = p1.x(), y1 = p1.y();

	i32 mix = qMin(x0, x1);
	i32 miy = qMin(y0, y1);
	i32 max = qMax(x0, x1);
	i32 may = qMax(y0, y1);

	if (offset == QVector2D() && scale.x() <= 1) {
		mix = qBound(0, mix, i32(texture.getWidth()));
		max = qBound(0, max, i32(texture.getWidth()));
		miy = qBound(0, miy, i32(texture.getHeight()));
		may = qBound(0, may, i32(texture.getHeight()));
	}

	if (mix == max)
		max = mix + 1;

	if (miy == may)
		may = miy + 1;

	for(i32 x = mix; x < max; ++x)
		for (i32 y = miy; y < may; ++y) {
			QPoint point = pixelToTexture(QPoint(x, y));
			texture.store(u16(point.x()), u16(point.y()), getSelectedPalette());
		}
}

//TODO: Tooltips
//TODO: Toolbar for palette

void TileRenderer::mouseMoveEvent(QMouseEvent *e) {

	//TODO: Set cursor icon for painting
	//TODO: Set painter icon for painting

	if (!isMouseDown) {
		setFocus();
		return;
	}

	if (!texture.getWidth() || !isMouseDown)
		return;

	QPoint next = e->pos();

	if (next.x() < 0 || next.y() < 0 || next.x() >= width() || next.y() >= height())
		return;

	if (ctrl) {
		QPoint dif = prev - next;
		offset += QVector2D(f32(dif.x()) / texture.getWidth(), f32(dif.y()) / texture.getHeight()) * scale;
		prev = next;
		return;
	}

	if (!editable || tool != TilePaintTool::BRUSH || alt)
		return;

	drawLine(globalToPixel(prev), globalToPixel(next));
	prev = next;

	updateTexture();
}

void TileRenderer::updateTexture() {
	destroyGTexture();
	setupGTexture();
}

void TileRenderer::reset() {
	texture = {};
	paletteTexture = {};
	destroyGTexture();
}

void TileRenderer::updateScale() {

	if (texture.getWidth() == 0)
		setMinimumSize(256, 256);

	else setMinimumSize(texture.getWidth(), texture.getHeight());

	f32 scaleX = f32(width()) / texture.getWidth();
	f32 scaleY = f32(height()) / texture.getHeight();

	if (scaleX == scaleY)
		return;

	if (scaleX < scaleY)
		scaleY = scaleX;

	else scaleX = scaleY;

	i32 sizeX = i32(scaleX * texture.getWidth());
	i32 sizeY = i32(scaleY * texture.getHeight());

	resize(sizeX, sizeY);
	updateGeometry();
}

//TODO: Center tile renderer and palette render
//TODO: Center toolbar

void TileRenderer::resizeEvent(QResizeEvent *e) {

	QOpenGLWidget::resizeEvent(e);

	if (!texture.getWidth())
		return;

	QSize size = e->size();

	f32 scaleX = f32(size.width()) / texture.getWidth();
	f32 scaleY = f32(size.height()) / texture.getHeight();

	if (scaleX == scaleY)
		return;

	if (scaleX < scaleY)
		scaleY = scaleX;

	else scaleX = scaleY;

	i32 sizeX = i32(scaleX * texture.getWidth());
	i32 sizeY = i32(scaleY * texture.getHeight());

	resize(sizeX, sizeY);		//TODO: This can cause an infinite loop
	updateGeometry();
}