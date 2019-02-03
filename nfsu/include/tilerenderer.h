#pragma once
#include <QtWidgets/qopenglwidget.h>
#include "texture.h"

namespace nfsu {

	class TileRenderer : public QOpenGLWidget {

	public:

		TileRenderer(u32 scale = 1);
		~TileRenderer();

		void setTexture(nfs::Texture2D texture);
		nfs::Texture2D getTexture();

		void initializeGL() override;
		void paintGL() override;

	protected:

		void destroyGTexture();
		void setupGTexture();

	private:

		nfs::Texture2D texture;

		u32 scale;

		GLuint shader, quadVAO, quadVBO, gtexture,
			textureLocation, widthLocation, heightLocation, tiledLocation;

	};

}