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
		void setGridColor(QColor color);
		void setGridSize(f32 percToPix);

		void setEditable(bool b);

		void initializeGL() override;
		void paintGL() override;
		void mousePressEvent(QMouseEvent *e) override;

		void set(QPoint p0, u32 color);

		QPoint globalToTexture(QPoint pos);

		//Re-initialize texture & repaint
		void updateTexture();

		///selected palette index

		u8 getPrimary();
		u8 getSecondary();

	protected:

		void destroyGTexture();
		void setupGTexture();

	private:

		nfs::Texture2D texture;

		u8 primary = 0, secondary = 0;
		bool showGrid = true, editable = true;

		i32 gridColor = 0xFFFFFF;
		f32 gridSize = 0.075f;

		QGLShaderProgram shader;
		QGLBuffer quad;

		QOpenGLTexture *paletteTexture = nullptr;

	};

}