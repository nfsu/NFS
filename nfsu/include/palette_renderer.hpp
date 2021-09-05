#pragma once
#include "texture.hpp"

#pragma warning(push, 0)
	#include <QtWidgets/qopenglwidget.h>
	#include <QtOpenGL/qglshaderprogram.h>
	#include <QtOpenGL/qglbuffer.h>
#pragma warning(pop)

class QPoint;
class QOpenGLTexture;

namespace nfsu {

	class PaletteRenderer : public QOpenGLWidget {

	public:

		PaletteRenderer();
		~PaletteRenderer();

		void setTexture(nfs::Texture2D texture);
		nfs::Texture2D getTexture();

		QOpenGLTexture *getGPUTexture();

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

		//selected palette index

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

		QOpenGLTexture *paletteTexture = nullptr;
	};
}