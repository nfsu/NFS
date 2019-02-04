#include "tilerenderer.h"
#include <QtGui/qevent.h>
using namespace nfsu;
using namespace nfs;

#define check(errCode, ...) if(__VA_ARGS__) throw std::runtime_error(errCode);

//Shader source

char const *vertShader = 

	"#version 330 core\r\n"

	"layout(location=0) in vec2 pos;"

	"out vec2 uv;"

	"void main() {"
		"uv = pos;"
		"gl_Position = vec4(pos * 2 - 1, 0, 1);"
	"}";

char const *fragShader =

	"#version 330 core\r\n"

	"in vec2 uv;"

	"uniform int width;"
	"uniform int height;"
	"uniform int size;"
	"uniform int tiled;"
	"uniform usampler1D tiledTexture;"
	"uniform int flags;"
	"uniform sampler2D paletteTexture;"

	"out vec4 color;"

	"void main() {"

		//Convert from pixel to tiled pixel space

		"ivec2 px = ivec2(uv * vec2(width, height));"
		"ivec2 tile = px % tiled;"
		"ivec2 tiles = px / tiled;"
		"int pos = (tiles.x + tiles.y * width / tiled) * tiled * tiled + tile.y * tiled + tile.x;"

		//Convert from pixel index to buffer index

		"int mod2x4 = (pos % 2) * 4;"

		"if((flags & 1) != 0)"
			"pos /= 2;"

		"int val = int(texture(tiledTexture, float(pos) / size).r);"

		"if((flags & 1) != 0)"
			"val = (val & (0xF << mod2x4)) >> mod2x4;"

		//Convert from palette index to color

		"if((flags & 2) != 0)"
			"color = vec4(texture(paletteTexture, vec2(val & 0xF, (val & 0xF0) >> 4) / vec2(16, 16)).rgb, 1);"
		"else "
			"color = vec4(vec2((val & 0xF) << 4, val & 0xF0) / vec2(255, 255), 0, 1);"
	"}";

//Quad

const float quad[] = {
	0,0, 0,1, 1,1,
	1,1, 1,0, 0,0
};

//Setup renderer

TileRenderer::TileRenderer(u32 scale) : texture(nullptr, 0, 0, 0), scale(scale) {
	setFixedSize(256 * scale, 256 * scale);
}

void TileRenderer::initializeGL() {

	//Setup shader

	check("Couldn't compile vertex shader", !shader.addShaderFromSourceCode(QGLShader::Vertex, vertShader));
	check("Couldn't compile fragment shader", !shader.addShaderFromSourceCode(QGLShader::Fragment, fragShader));
	check("Couldn't link shader", !shader.link());

	//Setup vbo and vao

	quadVBO = QGLBuffer(QGLBuffer::VertexBuffer);
	quadVBO.create();
	quadVBO.bind();
	quadVBO.allocate(quad, sizeof(quad));

}

//Get texture

Texture2D TileRenderer::getTexture() {
	return texture;
}

//Clean up resources

TileRenderer::~TileRenderer() {
	shader.deleteLater();
	quadVBO.destroy();
	destroyGTexture();
}

//Texture functions

void TileRenderer::setTexture(Texture2D tex) {
	texture = tex;
	destroyGTexture();
	setupGTexture();
}

void TileRenderer::destroyGTexture() {

	if (tiledTexture == nullptr) return;

	tiledTexture->destroy();
	delete tiledTexture;
}

void TileRenderer::setupGTexture() {

	tiledTexture = new QOpenGLTexture(QOpenGLTexture::Target1D);
	tiledTexture->setMinMagFilters(QOpenGLTexture::NearestMipMapNearest, QOpenGLTexture::Nearest);
	tiledTexture->setFormat(QOpenGLTexture::R8U);
	tiledTexture->setSize(texture.getDataSize());
	tiledTexture->allocateStorage(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt8);
	tiledTexture->setData(QOpenGLTexture::Red_Integer, QOpenGLTexture::UInt8, texture.getPtr());

	repaint();

}

//Settings

void TileRenderer::usePalette(bool b) {
	palette = b;
	repaint();
}

void TileRenderer::setEditable(bool b) {
	editable = b;
}

void TileRenderer::setCursor(u32 i) {
	idx = i;
}

void TileRenderer::setCursorSize(u32 scale) {
	cursorSize = scale;
}

//Render calls

void TileRenderer::paintGL() {

	shader.bind();

	if(tiledTexture)
		tiledTexture->bind(0);

	//if(paletteTexture)
	//	paletteTexture->bind(1);

	shader.setUniformValue("tiledTexture", 0);
	shader.setUniformValue("paletteTexture", 1);
	shader.setUniformValue("width", (i32) texture.getWidth());
	shader.setUniformValue("height", (i32) texture.getHeight());
	shader.setUniformValue("tiled", (i32) texture.getTiles());
	shader.setUniformValue("size", (i32) texture.getDataSize());
	shader.setUniformValue("flags", 
		(texture.getType() == TextureType::R4 ? 1 : 0) |
		(palette ? 2 : 0)
	);

	quadVBO.bind();

	int pos = shader.attributeLocation("pos");
	shader.enableAttributeArray(pos);
	shader.setAttributeBuffer(pos, GL_FLOAT, 0, 2);

	glDrawArrays(GL_TRIANGLES, 0, 6);

}

//Drawing

void TileRenderer::mousePressEvent(QMouseEvent *e) {

	if (e->button() == Qt::MouseButton::LeftButton) {
		isMouseDown = true;
		mouseMoveEvent(e);
	}

	//TODO: Fill tile? Or square editor?

}

void TileRenderer::mouseReleaseEvent(QMouseEvent *e) {

	if (e->button() == Qt::MouseButton::LeftButton)
		isMouseDown = false;

	//TODO: Line editor?

}

void TileRenderer::mouseMoveEvent(QMouseEvent *e) {

	if (texture.getWidth() == 0 || !isMouseDown || !editable)
		return;

	//TODO: Stroke if there was a last position?

	float px = (float) e->x() / width(), py = (float) e->y() / height();

	i32 x = i32(px * texture.getWidth());
	i32 y = i32((1 - py) * texture.getHeight());

	i32 sx = x - cursorSize / 2;
	i32 sy = y - cursorSize / 2;
	i32 ex = x + cursorSize / 2;
	i32 ey = y + cursorSize / 2;

	if (sx == ex) {
		++ex;
		++ey;
	}

	for(i32 i = sx; i < ex; ++i)
		for(i32 j = sy; j < ey; ++j){

			if (i < 0 || j < 0 || i >= texture.getWidth() || j >= texture.getHeight())
				continue;

			texture.store(i, j, idx);
		}

	destroyGTexture();
	setupGTexture();
	repaint();
}