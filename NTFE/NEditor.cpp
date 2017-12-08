#include "NEditor.h"
#include <NTypes2.h>

void logGLErrors() {

	GLenum error;
	if ((error = glGetError()) != GL_NO_ERROR) {
		switch (error) {
		case GL_INVALID_ENUM:
			printf("%s\n", "Invalid enum!");
			break;
		case GL_INVALID_VALUE:
			printf("%s\n", "Invalid value!");
			break;
		case GL_INVALID_OPERATION:
			printf("%s\n", "Invalid operation!");
			break;
		case GL_STACK_OVERFLOW:
			printf("%s\n", "Stack overflow!");
			break;
		case GL_OUT_OF_MEMORY:
			printf("%s\n", "Out of memory!");
			break;
		case GL_TABLE_TOO_LARGE:
			printf("%s\n", "Table to large!");
			break;
		default:
			printf("Unknown error!\n");
			break;
		}
	}

}

GLuint makeShader(GLenum type, const char *shaderSrc) {
	GLuint sh = glCreateShader(type);

	if (sh == 0) return 0;

	glShaderSource(sh, 1, &shaderSrc, nullptr);
	glCompileShader(sh);

	GLint hasErrors;

	glGetShaderiv(sh, GL_COMPILE_STATUS, &hasErrors);
	if (!hasErrors) {
		GLint RetinfoLen = 0;
		glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1) {
			char* infoLog = (char*)malloc(sizeof(char) * RetinfoLen);
			glGetShaderInfoLog(sh, RetinfoLen, NULL, infoLog);
			printf("%s", ("Error compiling this shader:\n" + std::string(infoLog) + "\n").c_str());
			free(infoLog);
		}
		glDeleteShader(sh);
		return 0;
	}
	return sh;
}

GLuint makeShader(std::string vert, std::string frag) {

	GLuint program = glCreateProgram();

	GLuint sVert, sFrag;

	if ((sVert = makeShader(GL_VERTEX_SHADER, vert.c_str())) == 0)
		return 0;

	if ((sFrag = makeShader(GL_FRAGMENT_SHADER, frag.c_str())) == 0)
		return 0;

	glAttachShader(program, sVert);
	glAttachShader(program, sFrag);

	glDeleteShader(sVert);
	glDeleteShader(sFrag);

	glLinkProgram(program);

	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLint RetinfoLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1) {
			GLchar* infoLog = (GLchar*)malloc(sizeof(char) * RetinfoLen);
			glGetProgramInfoLog(program, RetinfoLen, nullptr, infoLog);
			printf("%s", ("Error linking program:\n" + std::string(infoLog) + "\n").c_str());
		}
	}

	return program;
}

struct Position2D { f32 x, y; };

template<u32 i> GLuint makeVBO2D(Position2D (&posi)[i]) {

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(posi), &posi, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	logGLErrors();

	return vbo;
}

void destroyVBO(GLuint &vbo) {
	glDeleteBuffers(1, &vbo);
	vbo = 0;
}

void destroyShader(GLuint &shader) {
	glDeleteProgram(shader);
	shader = 0;
}

NEditor::NEditor(NEditorMode _mode, std::unordered_map<u32, GLuint> &_buffers, std::unordered_map<u32, Texture2D> &_textures): shader(0), vbo(0), mode(_mode), buffers(_buffers), textures(_textures) { }

void NEditor::initializeGL() {

	glewInit();

	std::string vert =
		"#version 330 core\r\n"
		"in layout(location=0) vec2 position;"

		"out vec2 uv;"

		"void main() {"
		"	gl_Position = vec4(position, 0, 1);"
		"	vec2 tuv = position * 0.5 + 0.5;"
		"	uv = vec2(tuv.x, 1 - tuv.y);"
		"}";

	std::string header = 
		"#version 330 core\r\n"
		"in vec2 uv;"
		"out vec4 color;";

	std::string palette =
		"uniform sampler2D palette;"
		"vec3 samplePalette(vec2 uv){ return texture(palette, vec2(uv.x, uv.y)).rgb; }";

	std::string tilemap =
		"uniform usampler2D tilemap;"

		"uniform uint isFourBit;"
		"uniform uint tiling;"
		"uniform uvec2 dim;"

		"vec3 sampleTilemap(vec2 uv) {"
		"	uvec2 pix = uvec2(uv * vec2(dim));"

		"	if(tiling != uint(0)) {"
		"		uvec2 tile = pix / tiling;"
		"		uvec2 tiles = dim / tiling;"
		"		uvec2 pixel = pix % tiling;"
		"		uint off = (tile.x + tile.y * tiles.x) * tiling * tiling + pixel.x + pixel.y * tiling;"
		"		uvec2 offset = uvec2(off % dim.x, off / dim.x);"
		"		uv = vec2(offset) / vec2(dim);"
		"	}"

		"	uint posi = uint(texture(tilemap, uv).r);"
		"	if(isFourBit != uint(0)) { \n"
		"		if(pix.x % uint(2) != uint(1))"
		"			posi &= uint(0xF);"
		"		else"
		"			posi = (posi & uint(0xF0)) >> uint(4);"
		"	}\n"

		"	uvec2 uv1 = uvec2(posi & uint(0xF), (posi & uint(0xF0)) >> uint(4));"
		"	vec2 uv2 = vec2(float(uv1.x) / 15.0, float(uv1.y) / 15.0);"
		"	return samplePalette(uv2);"
		"}";

	std::string map =
		"uniform usampler2D map;"
		"uniform uvec2 dim0;"

		"vec3 sampleMap(vec2 uv) {"

		"	uvec2 tile = uvec2(uv * dim0);"
		"	uvec2 pix = uvec2(uv * dim0 * tiling);"
		"	vec2 uv0 = vec2(tile) / vec2(dim0);"

		"	uvec2 tiles = dim / tiling;"

		"	uint sample = texture(map, uv0).r;"
		"	uint ptt = (sample & uint(0x0C00)) >> uint(10);"
		"	uint ptp = (sample & uint(0xF000)) >> uint(12);"
		"	uint ptm = (sample & uint(0x03FF)) >> uint( 0);"

		"	uvec2 pos = uvec2(ptm % tiles.x, ptm / tiles.x);"
		"	uvec2 off = pix % tiling;"

		"	if((ptt & uint(2)) != uint(0)) off.y = (tiling - uint(1)) - off.y;"
		"	if((ptt & uint(1)) != uint(0)) off.x = (tiling - uint(1)) - off.x;"

		"	uvec2 tpix = pos * tiling + off;"

		"	vec2 uv1 = vec2(tpix) / vec2(dim);"

		"	return sampleTilemap(uv1);"				///Palette??

		"}";

	if (mode == NEditorMode::PALETTE)
		shader = makeShader(vert, header + palette +
			"void main() {"
			"	color = vec4(samplePalette(uv), 1);"
			"}"

		);
	else if (mode == NEditorMode::TILEMAP) 
		shader = makeShader(vert, header + palette + tilemap + 
			"void main() {"
			"	color = vec4(sampleTilemap(uv), 1);"
			"}"

		);
	 else if (mode == NEditorMode::MAP) 
		shader = makeShader(vert, header + palette + tilemap + map +
			"void main() {"
			"	color = vec4(sampleMap(uv), 1);"
			"}"

		);

	Position2D data[4] = {
		{ -1, -1 },
		{ 1, -1 },
		{ 1, 1 },
		{ -1, 1 }
	};

	vbo = makeVBO2D(data);
}

NEditor::~NEditor() {
	destroyShader(shader);
	destroyVBO(vbo);
}

const char *getName(u32 i) {
	switch (i) {
	case 0:
		return "palette";
	case 1:
		return "tilemap";
	case 2:
		return "map";
	default:
		return "";

	}
}

void NEditor::paintGL() {

	glUseProgram(shader);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	for (auto i : buffers) {
		glActiveTexture(GL_TEXTURE0 + i.first);
		glBindTexture(GL_TEXTURE_2D, buffers[i.first]);

		GLuint loc = glGetUniformLocation(shader, getName(i.first));
		glUniform1i(loc, i.first);
	}

	if (mode == NEditorMode::TILEMAP || mode == NEditorMode::MAP) {

		u32 is4bit = (textures[1].tt & TextureType::B4) != 0;

		GLuint loc2 = glGetUniformLocation(shader, "isFourBit");
		glUniform1ui(loc2, is4bit);

		GLuint dim[2] = { textures[1].width, textures[1].height };
		GLuint loc3 = glGetUniformLocation(shader, "dim");
		glUniform2uiv(loc3, 1, dim);

		if (mode == NEditorMode::MAP) {
			GLuint dim0[2] = { textures[2].width, textures[2].height };
			GLuint loc4 = glGetUniformLocation(shader, "dim0");
			glUniform2uiv(loc4, 1, dim0);
		}

		GLuint loc4 = glGetUniformLocation(shader, "tiling");
		if ((textures[1].tt & TextureType::TILED8) != 0)
			glUniform1ui(loc4, 8);
	}

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}