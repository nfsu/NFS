#include "NEditor.h"

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

NEditor::NEditor(NEditorMode _mode, std::unordered_map<u32, GLuint> &_buffers): shader(0), vbo(0), mode(_mode), buffers(_buffers) { }

void NEditor::initializeGL() {

	glewInit();

	shader = makeShader(

		"#version 330 core\r\n"
		"attribute vec2 position;"

		"varying vec2 uv;"

		"void main() {"
		"	gl_Position = vec4(position, 0, 1);"
		"	vec2 tuv = position * 0.5 + 0.5;"
		"	uv = vec2(tuv.x, 1 - tuv.y);"
		"}"
		,

		"#version 330 core\r\n"
		"uniform sampler2D t;"

		"varying vec2 uv;"

		"void main() {"
		"	gl_FragColor = vec4(texture(t, uv).rgb, 1);"
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

void NEditor::paintGL() {

	glUseProgram(shader);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	if (mode == NEditorMode::PALETTE) 
		glBindTexture(GL_TEXTURE_2D, buffers[0]);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}