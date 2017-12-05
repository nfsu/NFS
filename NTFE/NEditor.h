#pragma once
#include <Generic.h>
#include <GL/glew.h>
#include <qopenglwidget.h>
#include <unordered_map>

enum class NEditorMode {

	PALETTE, TILEMAP, MAP,

	BINARY, TEXT,

	SOUND,

	MODEL

};

class NEditor : public QOpenGLWidget {

public:

	NEditor(NEditorMode mode, std::unordered_map<u32, GLuint> &buffers);
	~NEditor();

protected:
	
	void initializeGL() override;
	void paintGL() override;

public:

	GLuint shader, vbo;
	NEditorMode mode;
	
	std::unordered_map<u32, GLuint> &buffers;
};

void logGLErrors();