#pragma once
#include <QtWidgets/qopenglwidget.h>
#include <QtOpenGL/qglshaderprogram.h>
#include <QtOpenGL/qglbuffer.h>
#include <QtGui/qopengltexture.h>
#include "texture.h"

namespace nfsu {

	class PaletteRenderer : public QOpenGLWidget {

	public:

		~PaletteRenderer();

		void setTexture(nfs::Texture2D texture);
		nfs::Texture2D getTexture();

		QOpenGLTexture *getGPUTexture();
		QGLBuffer getQuad();

		void setShowGrid(bool b);
		void setEditable(bool b);

		void initializeGL() override;
		void paintGL() override;
		void mousePressEvent(QMouseEvent *e) override;

		void set(QPoint p0, u32 color);

		QPoint globalToTexture(QPoint pos);

		//Re-initialize texture & repaint
		void refresh();

		///selected palette index

		u8 getPrimary();
		u8 getSecondary();

	protected:

		void destroyGTexture();
		void setupGTexture();

	private:

		nfs::Texture2D texture;

		u8 primary = 0, secondary = 0;
		bool showGrid = false, editable = true;

		QGLShaderProgram shader;
		QGLBuffer quad;

		QOpenGLTexture *paletteTexture = nullptr;

	};

}