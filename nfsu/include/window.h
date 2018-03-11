#pragma once
#include <QtWidgets/qwidget.h>
#include <filesystem.h>

namespace nfsu {

	class Window : public QWidget {

	public:

		Window();
		~Window();

		void load();

		void load(QString file);
		void reload();

		void documentation();

		void write(QString file);
		void exportPatch(QString file);
		void importPatch(QString file);

	private:

		QString file;
		Buffer rom;
		nfs::FileSystem fileSystem;

	};

}