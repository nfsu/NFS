#pragma once
#include <QtWidgets/qwidget.h>
#include <filesystem.h>

namespace nfsu {

	class Window : public QWidget {

	public:

		Window();
		~Window();

		///UI actions

		void setupUI();
		void setupLayout();
		void setupToolbar();

		///File actions

		void load();
		void load(QString file);

		void reload();

		void write();
		void write(QString file);

		void exportPatch();
		void exportPatch(QString file);

		void importPatch();
		void importPatch(QString file);

		//void findFiles();

		///View actions
		void restore();

		///Options actions

		//void preferences();

		///Help actions

		void documentation();

	private:

		QString file;
		Buffer rom;
		nfs::FileSystem fileSystem;


		QLayout *layout;

	};

}