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

		TileRenderer();
		~TileRenderer();

		void setTexture(nfs::Texture2D texture);
		nfs::Texture2D getTexture();

		void setPalette(nfs::Texture2D texture);
		nfs::Texture2D getPalette();

		void setUsePalette(bool b);
		bool getUsePalette();

		void setEditable(bool b);
		
		void setCursorSize(u32 size);

		//If the image is 4-bit, use as y offset into palette
		void setPaletteOffset(u8 y);

		void setPaintTool(TilePaintTool tool);

		void initializeGL() override;
		void paintGL() override;
		void mouseMoveEvent(QMouseEvent *e) override;
		void mousePressEvent(QMouseEvent *e) override;
		void wheelEvent(QWheelEvent *e) override;
		void mouseReleaseEvent(QMouseEvent *e) override;
		void keyPressEvent(QKeyEvent *e) override;
		void keyReleaseEvent(QKeyEvent *e) override;

		void drawPoint(QPoint pixelSpace);
		void drawLine(QPoint pixelSpace, QPoint pixelSpace0);
		void drawSquare(QPoint pixelSpace, QPoint pixelSpace0);
		void fill(QPoint textureSpace);

		u32 get(QPoint p0);

		QPoint globalToTexture(QPoint pos);
		QPoint globalToPixel(QPoint pos);
		QPoint pixelToTexture(QPoint pos);

		u32 getSelectedPalette();

		//Re-initialize texture & repaint
		void updateTexture();

		void reset();
		void updateScale();

		void resizeEvent(QResizeEvent *e) override;

	protected:

		void setupGTexture();
		void destroyGTexture();

	private:


		u32 cursorSize = 1;

		bool usePalette = true, editable = true, isMouseDown = false,
			isLeft = false, ctrl = false, shift = false, alt = false;

		TilePaintTool tool = TilePaintTool::BRUSH;

		u8 yOffset = 0;

		nfs::Texture2D texture, palette;

		QPoint prev;
		QVector2D offset = { 0, 0 }, scale = { 1, 1 };

		QGLShaderProgram shader;
		QGLBuffer quad;

		QOpenGLTexture *tiledTexture = nullptr, *magicTexture = nullptr, *paletteTexture = nullptr;

	};

}