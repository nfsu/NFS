#pragma once

#include "NEditor.h"
#include <unordered_map>
#include <qsplitter.h>
#include <FileSystem.h>

class NEditors : public QWidget {

public:

	NEditors();
	~NEditors();

	void setTexture(u32 id, Texture2D t2d, nfs::FileSystemObject *fso);

	NEditor *add(QSplitter *parent, u32 mode);

private:

	std::vector<NEditor*> editors;

	std::unordered_map<u32, Texture2D> textures;
	std::unordered_map<u32, GLuint> buffers;
	std::unordered_map<u32, nfs::FileSystemObject*> files;
};