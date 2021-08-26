#pragma once
#include "file_system.hpp"
#include "info_window.hpp"
#include "resource_editor.hpp"

#pragma warning(push, 0)
	#include <QtWidgets/qproxystyle.h>
	#include <QtWidgets/qmainwindow.h>
#pragma warning(pop)

class QSplitter;
class QTabWidget;

namespace nfsu {

	class NExplorer;
	class NExplorerView;

	class StopAlt : public QProxyStyle {

	public:

		int styleHint(
			StyleHint stylehint, 
			const QStyleOption *opt, const QWidget *widget, 
			QStyleHintReturn *returnData
		) const override;
	};

	class Window : public QMainWindow {

	public:

		Window();
		~Window();

		//UI actions

		void setupUI();
		void setupLayout();
		void setupToolbar();
		void setupExplorer();
		void setupInfoWindow();
		void setupTabs(QLayout *layout);

		//Toolbar actions

			//File actions

			void _load();
			void load(const QString &file);

			void _reload();
			void reload();

			void _write();
			void write(const QString &file);

			void _exportPatch();
			void exportPatch(const QString &file);

			void _importPatch();
			void importPatch(const QString &file);

			void findFile();
			void filterFiles();
			void orderFiles();

			//View actions

			void restore();
			void customize();

			//Options actions

			void shortcuts();
			void preferences();

			//Help actions

			void documentation();
			void about();

		//Right click resource actions

			void activateResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao, const QPoint &point);

			void viewResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void viewData(Buffer buf);
			void exportResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void importResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void info(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);

			void inspect(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void inspectFolder(nfs::FileSystemObject &fso);

	private:

		QString currentFile{};
		Buffer rom;
		nfs::FileSystem fileSystem;

		i32 selectedId = 0;

		NExplorer *explorer = nullptr;
		NExplorerView *explorerView;
		InfoWindow *infoWindow = nullptr;

		QLayout *layout = nullptr, *rightLayout;
		QWidget *right, *central;
		QSplitter *splitter, *left;

		List<ResourceEditor*> editors;
		ResourceEditor *selected;
		QTabWidget *tabs;
	};
}