#pragma once
#include <QtWidgets/qopenglwidget.h>
#include <QtOpenGL/qglshaderprogram.h>
#include <QtOpenGL/qglbuffer.h>
#include <QtGui/qopengltexture.h>
#include "texture.h"

namespace nfsu {

	enum class TilePaintTool {
		BRUSH, LINE, SQUARE, FILL
	};

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

		void setPaintTool(TilePaintTool tool);

		void initializeGL() override;
		void paintGL() override;
		void mouseMoveEvent(QMouseEvent *e) override;
		void mousePressEvent(QMouseEvent *e) override;
		void mouseReleaseEvent(QMouseEvent *e) override;

		void drawPoint(QPoint point, u32 size = 0 /* uses cursorSize by default */);
		void drawLine(QPoint p0, QPoint p1, u32 size = 0 /* uses cursorSize by default */);
		void drawSquare(QPoint p0, QPoint p2);
		void fill(QPoint p0);

		void fill(i32 x, i32 y, u32 mask);

		QPoint globalToTexture(QPoint pos);

		//Re-initialize texture & repaint
		void refresh();

	protected:

		void destroyGTexture();
		void setupGTexture();

	private:

		nfs::Texture2D texture;

		u32 scale, idx = 0, cursorSize = 1;
		bool palette = false, editable = true, isMouseDown = false;
		
		TilePaintTool tool = TilePaintTool::BRUSH;

		QPoint prev;

		QGLShaderProgram shader;
		QGLBuffer quadVBO;

		QOpenGLTexture *tiledTexture = nullptr;

	};

}