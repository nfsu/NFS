#pragma once
#include <QtWidgets/qopenglwidget.h>
#include <QtOpenGL/qglshaderprogram.h>
#include <QtOpenGL/qglbuffer.h>
#include <QtGui/qopengltexture.h>
#include "texture.h"

namespace nfsu {

	class TileRenderer : public QOpenGLWidget {

	public:

		TileRenderer(u32 scale = 1);
		~TileRenderer();

		void setTexture(nfs::Texture2D texture);
		nfs::Texture2D getTexture();

		void usePalette(bool b);
		void setEditable(bool b);
		
		void setCursor(u32 idx);
		void setCursorSize(u32 size);

		void initializeGL() override;
		void paintGL() override;
		void mouseMoveEvent(QMouseEvent *e) override;
		void mousePressEvent(QMouseEvent *e) override;
		void mouseReleaseEvent(QMouseEvent *e) override;

	protected:

		void destroyGTexture();
		void setupGTexture();

	private:

		nfs::Texture2D texture;

		u32 scale, idx = 0, cursorSize = 5;
		bool palette = false, editable = true, isMouseDown = false;
		
		QGLShaderProgram shader;
		QGLBuffer quadVBO;

		QOpenGLTexture *tiledTexture = nullptr;

	};

}