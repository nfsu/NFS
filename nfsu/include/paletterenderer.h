#pragma once
#include <QtWidgets/qopenglwidget.h>
#include <QtOpenGL/qglshaderprogram.h>
#include <QtOpenGL/qglbuffer.h>
#include <QtGui/qopengltexture.h>
#include "texture.h"

namespace nfsu {

	class PaletteRenderer : public QOpenGLWidget {

	public:

		static constexpr float quadData[] = {
			0,0, 0,1, 1,1,
			1,1, 1,0, 0,0
		};

		PaletteRenderer();
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
		void mouseMoveEvent(QMouseEvent *e) override;
		void keyPressEvent(QKeyEvent *e) override;

		void set(QPoint p0, u32 color);
		u32 get(QPoint p0);

		QPoint globalToTexture(QPoint pos);

		//Re-initialize texture & repaint
		void updateTexture();

		void reset();

		void resizeEvent(QResizeEvent *e) override;

		///selected palette index

		u16 getPrimary();

	protected:

		void destroyGTexture();
		void setupGTexture();

	private:

		nfs::Texture2D texture;

		u8 primary = 0;
		bool showGrid = false, editable = true;

		i32 gridColor = 0xFFFFFF;
		f32 gridSize = 0.2222f;

		QGLShaderProgram shader;
		QGLBuffer quad;

		QOpenGLTexture *paletteTexture = nullptr;

	};

}