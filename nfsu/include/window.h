#pragma once
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qsplitter.h>
#include <QtWidgets/qtabwidget.h>
#include "filesystem.h"
#include "infowindow.h"
#include "resourceeditor.h"

namespace nfsu {

	class NExplorer;
	class NExplorerView;

	class Window : public QWidget {

	public:

		Window();
		~Window();

		///UI actions

		void setupUI();
		void setupLayout();
		void setupToolbar();
		void setupExplorer();
		void setupInfoWindow();
		void setupTabs(QLayout *layout);

		///Toolbar actions

			///File actions

			void load();
			void load(QString file);

			void reload();
			void reloadButton();

			void write();
			void write(QString file);

			void exportPatch();
			void exportPatch(QString file);

			void importPatch();
			void importPatch(QString file);

			void findFile();
			void filterFiles();
			void orderFiles();

			///View actions

			void restore();
			void customize();

			///Help actions

			void documentation();
			void about();

		///Right click resource actions

			void activateResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao, const QPoint &point);

			void viewResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void viewData(Buffer buf);
			void exportResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void importResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void info(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);

			void inspect(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void inspectFolder(nfs::FileSystemObject &fso);

	private:

		QString file;
		Buffer rom;
		nfs::FileSystem fileSystem;

		i32 selectedId = 0;

		NExplorer *explorer = nullptr;
		NExplorerView *explorerView;
		InfoWindow *fileInspect = nullptr;

		QLayout *layout = nullptr, *rightLayout;
		QWidget *right;
		QSplitter *splitter, *left;

		std::vector<ResourceEditor*> editors;
		ResourceEditor *selected;
		QTabWidget *tabs;

	};

}