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
		void setPrimaryHighlight(QColor color);
		void setSecondaryHighlight(QColor color);
		void setRowHighlight(QColor color);
		void setGridSize(f32 percToPix);
		void set4Bit(bool b);
		void setSelectedRow(u8 index);

		void setEditable(bool b);

		void initializeGL() override;
		void paintGL() override;
		void mousePressEvent(QMouseEvent *e) override;

		void set(QPoint p0, u32 color);
		u32 get(QPoint p0);

		QPoint globalToTexture(QPoint pos);

		//Re-initialize texture & repaint
		void updateTexture();

		void reset();

		///selected palette index

		u8 getPrimary();
		u8 getSecondary();

	protected:

		void destroyGTexture();
		void setupGTexture();

	private:

		nfs::Texture2D texture;

		u8 primary = 0, secondary = 0, selectedRow = 0;
		bool showGrid = true, editable = true, use4bit = false;

		i32 gridColor = 0xFFFFFF, primaryHighlight = 0x00FF00, secondaryHighlight = 0xFF0000, rowHighlight = 0x0000FF;
		f32 gridSize = 0.1f;

		QGLShaderProgram shader;
		QGLBuffer quad;

		QOpenGLTexture *paletteTexture = nullptr;

	};

}