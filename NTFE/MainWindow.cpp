#include "MainWindow.h"
#include <qmenu.h>
#include <qmenubar.h>
#include <qlayout.h>
#include <qscrollarea.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include "NExplorer.h"
#include "InfoTable.h"
#include <qtreeview.h>
#include <qtableview.h>
#include <qsplitter.h>
#include <qheaderview.h>
using namespace nfs;

MainWindow::MainWindow() {

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open ROM"), "", tr("NDS File (*.nds)"));
	romData = readFile(fileName.toStdString());

	if (romData.data != nullptr) {

		rom = NType::readNDS(romData);

		if (rom.romHeaderSize == 16384) {

			setWindowTitle(QString("File System Utilities: ") + rom.title);
			NType::convert(rom, &fs);

			QVBoxLayout *layout;
			setLayout(layout = new QVBoxLayout);

			QMenuBar *toolbar;
			layout->addWidget(toolbar = new QMenuBar);

			QMenu *file;
			toolbar->addMenu(file = new QMenu("File"));
			file->setFont(QFont("Charcoal CY"));

			QMenu *edit;
			toolbar->addMenu(edit = new QMenu("Edit"));
			edit->setFont(QFont("Charcoal CY"));

			QMenu *view;
			toolbar->addMenu(view = new QMenu("View"));
			view->setFont(QFont("Charcoal CY"));

			QMenu *options;
			toolbar->addMenu(options = new QMenu("Options"));
			options->setFont(QFont("Charcoal CY"));

			QMenu *help;
			toolbar->addMenu(help = new QMenu("Help"));
			help->setFont(QFont("Charcoal CY"));

			QSplitter *vertical = new QSplitter(Qt::Vertical);

			QSplitter *row1 = new QSplitter(Qt::Horizontal);
			/*QSplitter *row2 = new QSplitter(Qt::Horizontal);
			QSplitter *row3 = new QSplitter(Qt::Horizontal);*/

			///Row 1
			{
				NExplorer *model = new NExplorer(romData.data, fs);

				///Right

				NEditors *right = new NEditors();

				///Left
				QSplitter *left;
				{
					left = new QSplitter(Qt::Vertical);

					InfoTable *fileInfo;
					{
						auto contents = std::unordered_map<std::string, std::vector<std::string>>(2);

						contents["Types"].resize(10);

						contents["Types"][0] = "File";
						contents["Types"][1] = "Title";
						contents["Types"][2] = "Size";
						contents["Types"][3] = "Files";
						contents["Types"][4] = "Dirs";

						contents["Types"][5] = "Id";
						contents["Types"][6] = "Type";
						contents["Types"][7] = "Path";
						contents["Types"][8] = "Offset";
						contents["Types"][9] = "Size";

						contents["Values"].resize(10);

						contents["Values"][0] = fileName.toStdString();
						contents["Values"][1] = rom.title;
						contents["Values"][2] = QString::number(romData.size).toStdString() + " (" + QString::number(romData.size / 1024 / 1024).toStdString() + " MiB)";
						contents["Values"][3] = QString::number(fs.getFileCount()).toStdString();
						contents["Values"][4] = QString::number(fs.getFolderCount()).toStdString();

						contents["Values"][5] = "";
						contents["Values"][6] = "";
						contents["Values"][7] = "";
						contents["Values"][8] = "";
						contents["Values"][9] = "";

						fileInfo = new InfoTable(rom, fs, contents);

					}

					NExplorerView *tree = new NExplorerView(model, fileInfo, right);

					QTableView *table2 = new QTableView();
					table2->setModel(fileInfo);
					table2->horizontalHeader()->setStretchLastSection(true);

					table2->setMinimumSize(500, 325);

					left->addWidget(table2);
					left->addWidget(tree);

					left->setStretchFactor(0, 0);
					left->setStretchFactor(1, 1);

					left->setMinimumSize(500, 650);
				}

				row1->addWidget(left);
				row1->addWidget(right);

				row1->setStretchFactor(0, 0);
				row1->setStretchFactor(1, 1);
			}

			vertical->addWidget(row1);
			layout->addWidget(vertical);
		}
	}
}

MainWindow::~MainWindow() {
	deleteBuffer(&romData);
}