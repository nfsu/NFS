#pragma once
#include <QtWidgets/qopenglwidget.h>
#include <QtOpenGL/qglshaderprogram.h>
#include <QtOpenGL/qglbuffer.h>
#include "texture.h"

namespace nfsu {

	class TileRenderer : public QOpenGLWidget {

	public:

		TileRenderer(u32 scale = 1);
		~TileRenderer();

		void setTexture(nfs::Texture2D texture);
		nfs::Texture2D getTexture();

		void usePalette(bool b);

		void initializeGL() override;
		void paintGL() override;

	protected:

		void destroyGTexture();
		void setupGTexture();

	private:

		nfs::Texture2D texture;

		u32 scale;
		bool palette = true;
		
		QGLShaderProgram shader;
		QGLBuffer quadVBO;

		GLuint

			//OpenGL resources
			gtexture = 0, gpalette = 0;

	};

}