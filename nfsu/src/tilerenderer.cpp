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

	"uniform uint width;"
	"uniform uint height;"
	"uniform uint size;"
	"uniform uint tiled;"
	"uniform usampler1D tiledTexture;"
	"uniform uint flags;"
	"uniform sampler2D paletteTexture;"

	"out vec4 color;"

	"void main() {"

		//Convert from pixel to tiled pixel space

		"uvec2 px = uvec2(uv * vec2(width, height));"
		"uvec2 tile = px % tiled;"
		"uvec2 tiles = px / tiled;"
		"uint pos = (tiles.x + tiles.y * width / tiled) * tiled * tiled + tile.y * tiled + tile.x;"

		//Convert from pixel index to buffer index

		"uint mod2x4 = (pos % 2U) * 4U;"

		"if((flags & 0x1U) != 0U)"
			"pos /= 2U;"

		"uint val = texture(tiledTexture, float(pos) / size).r;"

		"if((flags & 0x1U) != 0U)"
			"val = (val & (0xFU << mod2x4)) >> mod2x4;"

		//Convert from palette index to color

		"if((flags & 0x2U) != 0U)"
			"color = vec4(texture(paletteTexture, vec2(val & 0xFU, (val & 0xF0U) >> 4U) / vec2(16, 16)).rgb, 1);"
		"else "
			"color = vec4(vec2((val & 0xFU) << 4U, val & 0xF0U) / vec2(255, 255), 0, 1);"
	"}";

//Quad

const float quad[] = {
	0,0, 0,1, 1,1,
	1,1, 1,0, 0,0
};

//OpenGL functions

PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;
PFNGLUNIFORM1UIPROC glUniform1ui = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;

//Setup renderer

TileRenderer::TileRenderer(u32 scale) : gtexture(0), texture(nullptr, 0, 0, 0), scale(scale) {
	setFixedSize(256 * scale, 256 * scale);
}

void TileRenderer::initializeGL() {

	//Setup OpenGL functions

	if (glActiveTexture == nullptr) {
		glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
		glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
		glUniform1ui = (PFNGLUNIFORM1UIPROC)wglGetProcAddress("glUniform1ui");
		glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
		glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
		glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
		glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
		glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
		glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
		glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
		glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
		glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
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

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);

	glEnableVertexAttribArray(0);
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0);

}

//Get texture

Texture2D TileRenderer::getTexture() {
	return texture;
}

//Clean up resources

TileRenderer::~TileRenderer() {

	if (quadVAO == 0)
		return;

	destroyGTexture();
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
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

	glClearColor(255, 0, 0, 255);
	glClear(GL_COLOR_BUFFER_BIT);

	shader.bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, gtexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gpalette);

	glUniform1i(textureLocation, 0);
	glUniform1i(paletteLocation, 1);

	glUniform1ui(widthLocation, texture.getWidth());
	glUniform1ui(heightLocation, texture.getHeight());
	glUniform1ui(tiledLocation, texture.getTiles());
	glUniform1ui(sizeLocation, texture.getDataSize());

	glUniform1ui(flagsLocation, 
		(texture.getType() == TextureType::R4 ? 1 : 0) | 
		(gpalette != gtexture && palette ? 2 : 0)
	);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

}