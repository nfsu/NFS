#pragma once
#include <QtWidgets/qopenglwidget.h>
#include <QtOpenGL/qglshaderprogram.h>
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

		GLuint 

			//OpenGL resources
			quadVBO = 0, gtexture, gpalette,

			//Uniforms
			textureLocation, paletteLocation, widthLocation, heightLocation,
			tiledLocation, flagsLocation, sizeLocation;

	};

}