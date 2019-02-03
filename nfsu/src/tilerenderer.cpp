#include "tilerenderer.h"
using namespace nfsu;
using namespace nfs;

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
	"uniform uint tiled;"
	"uniform usampler1D tex;"

	"out vec4 color;"

	"void main() {"
		"uvec2 px = uvec2(uv * vec2(width, height));"
		"uvec2 tile = px % tiled;"
		"uvec2 tiles = px / tiled;"
		"uint pos = (tiles.x + tiles.y * width / tiled) * tiled * tiled + tile.y * tiled + tile.x;"
		//"uint val = texture(tex, float(pos) / width / height).r;"
		"uint pixel = px.x + px.y * width;"
		"uint val = texture(tex, float(pos) / 2 / width / height).r;"
		"color = vec4(val % 2U, val / 2U % 2U, val / 4U % 2U, 1);"
	"}";

//Quad

const float quad[] = {
	0,0, 0,1, 1,1,
	1,1, 1,0, 0,0
};

//OpenGL functions

PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
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
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;

//Setup renderer

TileRenderer::TileRenderer(u32 scale) : gtexture(0), shader(0), texture(nullptr, 0, 0, 0), scale(scale) {
	setFixedSize(256 * scale, 256 * scale);
}

void TileRenderer::initializeGL() {

	//Setup OpenGL functions

	if (glCreateProgram == nullptr) {
		glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
		glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
		glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
		glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
		glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
		glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
		glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
		glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
		glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
		glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
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
		glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
		glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
	}

	//Setup shader

	GLint lengths[] = { strlen(vertShader), strlen(fragShader) };

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, (GLchar**)&vertShader, lengths);
	glCompileShader(vertex);

	GLint isCompiled = GL_FALSE;
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &isCompiled);
	
	if (!isCompiled) {

		GLint length = 0;
		glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> errorLog(length);
		glGetShaderInfoLog(vertex, length, &length, errorLog.data());

		printf("%s\n", errorLog.data());

		throw std::runtime_error("Couldn't compile vertex shader");
	}

	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, (GLchar**)&fragShader, lengths + 1);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &isCompiled);

	if (!isCompiled) {

		GLint length = 0;
		glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> errorLog(length);
		glGetShaderInfoLog(fragment, length, &length, errorLog.data());

		printf("%s\n", errorLog.data());

		throw std::runtime_error("Couldn't compile fragment shader");
	}

	shader = glCreateProgram();
	glAttachShader(shader, vertex);
	glAttachShader(shader, fragment);
	glLinkProgram(shader);
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	tiledLocation = glGetUniformLocation(shader, "tiled");
	textureLocation = glGetUniformLocation(shader, "tex");
	widthLocation = glGetUniformLocation(shader, "width");
	heightLocation = glGetUniformLocation(shader, "height");

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

	if (shader == 0)
		return;

	destroyGTexture();
	glDeleteProgram(shader);
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

	repaint();

}

//Render calls

void TileRenderer::paintGL() {

	glClearColor(255, 0, 0, 255);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, gtexture);

	glUniform1i(textureLocation, 0);
	glUniform1ui(widthLocation, texture.getWidth());
	glUniform1ui(heightLocation, texture.getHeight());
	glUniform1ui(tiledLocation, texture.getTiles());

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

}