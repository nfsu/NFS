#include "NEditor.h"
#include <NTypes2.h>
#include <qfiledialog.h>

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

NEditor::NEditor(u32 _mode, std::unordered_map<u32, GLuint> &_buffers, std::unordered_map<u32, Texture2D> &_textures, std::unordered_map<u32, nfs::FileSystemObject*> &_files, QPushButton *_actions[4]): shader(0), vbo(0), mode(_mode), buffers(_buffers), textures(_textures), files(_files) {
	for (u32 i = 0; i < 4; ++i) {
		actions[i] = _actions[i];
		connect(actions[i], &QPushButton::pressed, this, [this, i]() { this->action(i); });
	}
}

void NEditor::action(u32 i) {
	switch (i) {
	case 0:
		editors[mode].save(this);
		break;
	case 1:
		editors[mode].ex(this);
		break;
	case 2:
		editors[mode].load(this);
		break;
	default:
		editors[mode].import(this);
		break;
	}
}


std::unordered_map<u32, NEditorType> NEditor::editors = __editors();

std::unordered_map<u32, NEditorType> NEditor::__editors() {

	std::unordered_map<u32, NEditorType> map;

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
		"	if(isFourBit != uint(0)) {"
		"		if(pix.x % uint(2) != uint(1))"
		"			posi &= uint(0xF);"
		"		else"
		"			posi = (posi & uint(0xF0)) >> uint(4);"
		"	}"

		"	uvec2 uv1 = uvec2(posi & uint(0xF), (posi & uint(0xF0)) >> uint(4));"
		"	vec2 uv2 = vec2(float(uv1.x) / 15.0, float(uv1.y) / 15.0);"
		"	return samplePalette(uv2);"
		"}";

	std::string maps =
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

	map[(u32)NEditorMode::PALETTE].name = "palette";
	map[(u32)NEditorMode::PALETTE].prepareRender = [](NEditor *editor) {};
	map[(u32)NEditorMode::PALETTE].init = [vert, header, palette](NEditor *editor) -> GLuint {
		return makeShader(vert, header + palette +
			"void main() {"
			"	color = vec4(samplePalette(uv), 1);"
			"}"
		);
	};
	map[(u32)NEditorMode::PALETTE].onActivate = [](NEditor*) -> void {};
	map[(u32)NEditorMode::PALETTE].save = [](NEditor*) -> void {};
	map[(u32)NEditorMode::PALETTE].ex = [](NEditor *edit) -> void {
		QString fileName = QFileDialog::getSaveFileName(edit, tr("Export"), "", tr("PNG file (*.png);;NCLR (palette) file (*.NCLR)"));
		if (fileName.endsWith(".png", Qt::CaseInsensitive))
			writeTexture(edit->getBoundTexture(0), fileName.toStdString());
		else
			writeBuffer(edit->getBoundFile(0)->buffer, fileName.toStdString());
	};
	map[(u32)NEditorMode::PALETTE].load = [](NEditor*) -> void {};
	map[(u32)NEditorMode::PALETTE].import = [](NEditor*) -> void {};
	map[(u32)NEditorMode::PALETTE].activateButtons = [](NEditor *edit) -> bool {
		return edit->hasBoundTexture(0);
	};

	auto setupTiling = [](NEditor *n, u32 mode) {

		Texture2D tilemap = n->getBoundTexture(1);

		u32 is4bit = (tilemap.tt & TextureType::B4) != 0;

		GLuint loc2 = glGetUniformLocation(n->getShader(), "isFourBit");
		glUniform1ui(loc2, is4bit);

		GLuint dim[2] = { tilemap.width, tilemap.height };
		GLuint loc3 = glGetUniformLocation(n->getShader(), "dim");
		glUniform2uiv(loc3, 1, dim);

		if (mode == (u32)NEditorMode::MAP) {
			Texture2D map = n->getBoundTexture(2);
			GLuint dim0[2] = { map.width, map.height };
			GLuint loc4 = glGetUniformLocation(n->getShader(), "dim0");
			glUniform2uiv(loc4, 1, dim0);
		}

		GLuint loc4 = glGetUniformLocation(n->getShader(), "tiling");
		if ((tilemap.tt & TextureType::TILED8) != 0)
			glUniform1ui(loc4, 8);
	};

	map[(u32)NEditorMode::TILEMAP].name = "tilemap";
	map[(u32)NEditorMode::TILEMAP].prepareRender = [setupTiling](NEditor *editor) { setupTiling(editor, (u32)NEditorMode::TILEMAP); };
	map[(u32)NEditorMode::TILEMAP].init = [vert, header, palette, tilemap](NEditor *editor) -> GLuint {
		return makeShader(vert, header + palette + tilemap + 
			"void main() {"
			"	color = vec4(sampleTilemap(uv), 1);"
			"}"
		);
	};
	map[(u32)NEditorMode::TILEMAP].onActivate = [](NEditor*) -> void {};
	map[(u32)NEditorMode::TILEMAP].save = [](NEditor*) -> void {};
	map[(u32)NEditorMode::TILEMAP].ex = [](NEditor *edit) -> void {
		QString fileName = QFileDialog::getSaveFileName(edit, tr("Export"), "", tr("PNG file (*.png);;NCGR (tilemap) file (*.NCGR);;NCLR (palette) file (*.NCLR)"));

		if (fileName.endsWith(".png", Qt::CaseInsensitive)) {
			Texture2D result = convertPT2D({ edit->getBoundTexture(0), edit->getBoundTexture(1) });
			writeTexture(result, fileName.toStdString());
			deleteTexture(&result);
		} 
		else if (fileName.endsWith(".NCGR", Qt::CaseInsensitive)) 
			writeBuffer(edit->getBoundFile(1)->buffer, fileName.toStdString());
		else
			writeBuffer(edit->getBoundFile(0)->buffer, fileName.toStdString());
	};
	map[(u32)NEditorMode::TILEMAP].load = [](NEditor*) -> void {};
	map[(u32)NEditorMode::TILEMAP].import = [](NEditor*) -> void {};
	map[(u32)NEditorMode::TILEMAP].activateButtons = [](NEditor *edit) -> bool {
		return edit->hasBoundTexture(0) && edit->hasBoundTexture(1);
	};

	map[(u32)NEditorMode::MAP].name = "map";
	map[(u32)NEditorMode::MAP].prepareRender = [setupTiling](NEditor *editor) { setupTiling(editor, (u32)NEditorMode::MAP); };
	map[(u32)NEditorMode::MAP].init = [vert, header, palette, tilemap, maps](NEditor *editor) -> GLuint {
		return makeShader(vert, header + palette + tilemap + maps +
			"void main() {"
			"	color = vec4(sampleMap(uv), 1);"
			"}"
		);
	};
	map[(u32)NEditorMode::MAP].onActivate = [](NEditor*) -> void {};
	map[(u32)NEditorMode::MAP].save = [](NEditor*) -> void {};
	map[(u32)NEditorMode::MAP].ex = [](NEditor *edit) -> void {
		QString fileName = QFileDialog::getSaveFileName(edit, tr("Export"), "", tr("PNG file (*.png);;NSCR (map) file (*.NSCR);;NCGR (tilemap) file (*.NCGR);;NCLR (palette) file (*.NCLR)"));

		if (fileName.endsWith(".png", Qt::CaseInsensitive)) {
			Texture2D result = convertTT2D({ edit->getBoundTexture(0), edit->getBoundTexture(1), edit->getBoundTexture(2) });
			writeTexture(result, fileName.toStdString());
			deleteTexture(&result);
		}
		else if (fileName.endsWith(".NCGR", Qt::CaseInsensitive))
			writeBuffer(edit->getBoundFile(1)->buffer, fileName.toStdString());
		else if (fileName.endsWith(".NSCR", Qt::CaseInsensitive))
			writeBuffer(edit->getBoundFile(2)->buffer, fileName.toStdString());
		else
			writeBuffer(edit->getBoundFile(0)->buffer, fileName.toStdString());
	};
	map[(u32)NEditorMode::MAP].load = [](NEditor*) -> void {};
	map[(u32)NEditorMode::MAP].import = [](NEditor*) -> void {};
	map[(u32)NEditorMode::MAP].activateButtons = [](NEditor *edit) -> bool {
		return edit->hasBoundTexture(0) && edit->hasBoundTexture(1) && edit->hasBoundTexture(2);
	};

	return map;
}

void NEditor::activate() {
	bool b = editors[mode].activateButtons(this);

	for (u32 i = 0; i < 4; ++i)
		actions[i]->setEnabled(b);

	editors[mode].onActivate(this);
}

void NEditor::initializeGL() {

	glewInit();

	shader = editors[mode].init(this);

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

bool NEditor::addType(u32 i, NEditorType net) {
	if (editors.find(i) != editors.end())
	return false;

	editors[i] = net;
	return true;
}

u32 NEditor::getSize() { return (u32)editors.size(); }

GLuint NEditor::getShader() const { return shader; }
Texture2D NEditor::getBoundTexture(u32 i) const { return textures[i]; }
nfs::FileSystemObject *NEditor::getBoundFile(u32 i) const { return files[i]; }
bool NEditor::hasBoundTexture(u32 i) const {
	if (textures.find(i) == textures.end()) return false;
	return textures[i].size != 0;
}

void NEditor::paintGL() {

	glUseProgram(shader);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	for (auto i : buffers) {
		glActiveTexture(GL_TEXTURE0 + i.first);
		glBindTexture(GL_TEXTURE_2D, buffers[i.first]);

		GLuint loc = glGetUniformLocation(shader, editors[i.first].name.c_str());
		glUniform1i(loc, i.first);
	}

	editors[mode].prepareRender(this);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}