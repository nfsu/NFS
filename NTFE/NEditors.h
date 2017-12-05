#pragma once

#include "NEditor.h"
#include <unordered_map>

class NEditors : public QWidget {

public:

	NEditors();
	~NEditors();

	void setTexture(u32 id, Texture2D t2d);

private:

	std::vector<NEditor*> editors;

	std::unordered_map<u32, Texture2D> textures;
	std::unordered_map<u32, GLuint> buffers;
};