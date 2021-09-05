#pragma once
#include "texture.hpp"

#pragma warning(push, 0)
	#include <QtWidgets/qopenglwidget.h>
	#include <QtOpenGL/qglshaderprogram.h>
	#include <QtOpenGL/qglbuffer.h>
#pragma warning(pop)

class QOpenGLTexture;

namespace nfsu {

	class TilemapRenderer : public QOpenGLWidget {

	public:

		TilemapRenderer();
		~TilemapRenderer();

		void setTiles(nfs::Texture2D texture);
		inline nfs::Texture2D getTiles() const { return tiles; }

		void setTilemap(nfs::Texture2D texture);
		inline nfs::Texture2D getTilemap() const { return tilemap; }

		void setPalette(nfs::Texture2D texture);
		nfs::Texture2D getPalette();

		void setUsePalette(bool b);
		void setUseGrid(bool b);
		inline bool getUsePalette() const { return usePalette; }
		inline bool getUseGrid() const { return useGrid; }

		void setEditable(bool b);
		
		void setGridColor(QColor color);
		void setGridSize(f32 percToPix);

		void initializeGL() override;
		void paintGL() override;
		void mouseMoveEvent(QMouseEvent *e) override;
		void mousePressEvent(QMouseEvent *e) override;
		void wheelEvent(QWheelEvent *e) override;
		void mouseReleaseEvent(QMouseEvent *e) override;
		void keyPressEvent(QKeyEvent *e) override;
		void keyReleaseEvent(QKeyEvent *e) override;

		QPoint globalToTexture(QPoint pos);
		QPoint globalToPixel(QPoint pos);
		QPoint pixelToTexture(QPoint pos);

		//Re-initialize texture & repaint
		void updateTexture();

		void reset();
		void updateScale();

		void resizeEvent(QResizeEvent *e) override;

	protected:

		void setupGTexture();
		void destroyGTexture();

	private:


		i32 gridColor = 0xFFFFFF;
		f32 gridSize = 0.0625f;

		bool usePalette = 1, editable = 1, useGrid = 0, 
			 isMouseDown{}, isLeft{}, ctrl{}, shift{}, alt{};

		nfs::Texture2D tilemap, tiles, palette;

		QPoint prev;
		QVector2D offset = { 0, 0 }, scale = { 1, 1 };

		QGLShaderProgram shader;

		QOpenGLTexture *tilemapTexture = nullptr, *tileTexture = nullptr, *magicTexture = nullptr, *paletteTexture = nullptr;
	};
}