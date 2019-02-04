#include "tilerenderer.h"
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

//OpenGL functions

PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;

//Setup renderer

TileRenderer::TileRenderer(u32 scale) : gtexture(0), texture(nullptr, 0, 0, 0), scale(scale) {
	setFixedSize(256 * scale, 256 * scale);
}

void TileRenderer::initializeGL() {

	//Setup OpenGL functions

	if (glActiveTexture == nullptr) {
		glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
		glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	}

	//Setup shader

	check("Couldn't compile vertex shader", !shader.addShaderFromSourceCode(QGLShader::Vertex, vertShader));
	check("Couldn't compile fragment shader", !shader.addShaderFromSourceCode(QGLShader::Fragment, fragShader));
	check("Couldn't link shader", !shader.link());

	tiledLocation = shader.uniformLocation("tiled");
	textureLocation = shader.uniformLocation("tiledTexture");
	paletteLocation = shader.uniformLocation("paletteTexture");
	widthLocation = shader.uniformLocation("width");
	heightLocation = shader.uniformLocation("height");
	flagsLocation = shader.uniformLocation("flags");
	sizeLocation = shader.uniformLocation("size");

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
	//setFixedSize(tex.getWidth() * scale, tex.getHeight() * scale);
}

void TileRenderer::destroyGTexture() {

	if (gtexture == 0)
		return;

	glDeleteTextures(1, &gtexture);

}

void TileRenderer::setupGTexture() {

	glGenTextures(1, &gtexture);
	glBindTexture(GL_TEXTURE_1D, gtexture);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_R8UI, texture.getDataSize(), 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, texture.getPtr());

	gpalette = gtexture;	//TODO: Setup palette

	repaint();

}

void TileRenderer::usePalette(bool b) {

	palette = b;

	repaint();

}

//Render calls

void TileRenderer::paintGL() {

	shader.bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, gtexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gpalette);

	glUniform1i(textureLocation, 0);
	glUniform1i(paletteLocation, 1);

	glUniform1i(widthLocation, texture.getWidth());
	glUniform1i(heightLocation, texture.getHeight());
	glUniform1i(tiledLocation, texture.getTiles());
	glUniform1i(sizeLocation, texture.getDataSize());

	glUniform1i(flagsLocation, 
		(texture.getType() == TextureType::R4 ? 1 : 0) | 
		(gpalette != gtexture && palette ? 2 : 0)
	);

	quadVBO.bind();

	int pos = shader.attributeLocation("pos");
	shader.enableAttributeArray(pos);
	shader.setAttributeBuffer(pos, GL_FLOAT, 0, 2);

	glDrawArrays(GL_TRIANGLES, 0, 6);

}