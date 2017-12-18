#pragma once
#include <Generic.h>
#include <GL/glew.h>
#include <qopenglwidget.h>
#include <unordered_map>
#include <functional>
#include <unordered_map>
#include <qpushbutton.h>
#include <FileSystem.h>

enum class NEditorMode : u32 {

	PALETTE, TILEMAP, MAP,

	BINARY, TEXT,

	SOUND,

	MODEL

};

class NEditor;

struct NEditorType {
	
	std::string name;
	std::function<GLuint (NEditor*)> init;
	std::function<void (NEditor*)> prepareRender;

	std::function<bool (NEditor*)> activateButtons;
	std::function<void (NEditor*)> save, ex, load, import, onActivate;

};

class NEditor : public QOpenGLWidget {

public:

	NEditor(u32 mode, std::unordered_map<u32, GLuint> &buffers, std::unordered_map<u32, Texture2D> &textures, std::unordered_map<u32, nfs::FileSystemObject*> &files, QPushButton *actions[4]);
	~NEditor();

	GLuint getShader() const;
	Texture2D getBoundTexture(u32 i) const;
	nfs::FileSystemObject *getBoundFile(u32 i) const;

	bool hasBoundTexture(u32 i) const;

	void activate();
	void action(u32 i);

	static bool addType(u32 i, NEditorType net);
	static u32 getSize();


protected:
	
	void initializeGL() override;
	void paintGL() override;

	static std::unordered_map<u32, NEditorType> editors;
	static std::unordered_map<u32, NEditorType> __editors();

private:

	GLuint shader, vbo;
	u32 mode;
	
	std::unordered_map<u32, GLuint> &buffers;
	std::unordered_map<u32, Texture2D> &textures;
	std::unordered_map<u32, nfs::FileSystemObject*> &files;

	QPushButton *actions[4];
};

void logGLErrors();