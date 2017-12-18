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
#include <qpushbutton.h>
#include <qdesktopservices.h>
using namespace nfs;

void MainWindow::documentation() {
	QDesktopServices::openUrl(QUrl("https://github.com/Nielsbishere/NFS/blob/master/README.md"));
}

void MainWindow::findOnline() {
	QDesktopServices::openUrl(QUrl("https://docs.osomi.net/NFS"));
}

void MainWindow::changeFlag(u32 flag, bool triggered) {
	displayFlag = (displayFlag & ~(1 << flag)) | (triggered * (1 << flag));
	nex->setFlag(displayFlag);
	emit nex->dataChanged(QModelIndex(), QModelIndex());
}

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

			file->addAction("Save");
			file->addAction("Export");
			file->addAction("Load");
			file->addAction("Import");
			file->addAction("Find file");

			QMenu *view;
			toolbar->addMenu(view = new QMenu("View"));
			view->setFont(QFont("Charcoal CY"));

			view->addAction("Restore");
			view->addAction("Customize");

			QMenu *options;
			toolbar->addMenu(options = new QMenu("Options"));
			options->setFont(QFont("Charcoal CY"));

			QMenu *filter = options->addMenu("File filter");

			QAction *qa[7];
			u32 qaoff = 0;

			qa[qaoff++] = filter->addAction("Image files");
			qa[qaoff++] = filter->addAction("Model files");
			qa[qaoff++] = filter->addAction("Binary files");
			qa[qaoff++] = filter->addAction("Text files");
			qa[qaoff++] = filter->addAction("Audio files");
			qa[qaoff++] = filter->addAction("Archive files");
			qa[qaoff++] = filter->addAction("Unknown files");

			displayFlag = 0x7F;

			for (u32 i = 0; i < 7; ++i) {
				QAction *at = qa[i];
				at->setCheckable(true);
				at->setChecked(true);
				connect(qa[i], &QAction::triggered, this, [this, at, i]() { this->changeFlag(i, at->isChecked()); });
			}

			QMenu *order = options->addMenu("File order");

			QActionGroup *forder = new QActionGroup(this);
			QAction *fo[4];

			fo[0] = order->addAction("Numeric");
			fo[1] = order->addAction("Alphabetical");
			fo[2] = order->addAction("Size");
			fo[3] = order->addAction("Type");

			for (u32 i = 0; i < 4; ++i) {
				fo[i]->setCheckable(true);
				fo[i]->setActionGroup(forder);
			}

			fo[0]->setChecked(true);

			order->addSeparator();

			QActionGroup *adsc = new QActionGroup(this);

			QAction *asc = order->addAction("Ascending");
			QAction *dsc = order->addAction("Descending");
			asc->setCheckable(true);
			asc->setChecked(true);
			dsc->setCheckable(true);
			asc->setActionGroup(adsc);
			dsc->setActionGroup(adsc);

			options->addAction("Preferences");

			QMenu *help;
			toolbar->addMenu(help = new QMenu("Help"));
			help->setFont(QFont("Charcoal CY"));

			QAction *helpDoc = help->addAction("Documentation");
			connect(helpDoc, &QAction::triggered, this, &MainWindow::documentation);

			QAction *findOnline = help->addAction("Find online");
			connect(findOnline, &QAction::triggered, this, &MainWindow::findOnline);

			QSplitter *vertical = new QSplitter(Qt::Vertical);

			QSplitter *row1 = new QSplitter(Qt::Horizontal);
			/*QSplitter *row2 = new QSplitter(Qt::Horizontal);
			QSplitter *row3 = new QSplitter(Qt::Horizontal);*/

			///Row 1
			{
				NExplorer *model = new NExplorer(romData.data, fs);
				nex = model;

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

						fileInfo = new InfoTable(contents);

					}

					NExplorerView *tree = new NExplorerView(model, fileInfo, right);

					QWidget *top = new QWidget;
					QLayout *layout = new QVBoxLayout();
					top->setLayout(layout);

					{

						QTableView *table2 = new QTableView();
						table2->setModel(fileInfo);
						table2->horizontalHeader()->setStretchLastSection(true);

						table2->setMinimumSize(500, 325);

						layout->addWidget(table2);

						QWidget *fileOptions = new QWidget;
						QLayout *foLayout = new QHBoxLayout;
						fileOptions->setLayout(foLayout);

						QPushButton *qpb;
						foLayout->addWidget(qpb = new QPushButton("Export source"));
						qpb->setEnabled(false);
						foLayout->addWidget(qpb = new QPushButton("Export target"));
						qpb->setEnabled(false);
						foLayout->addWidget(qpb = new QPushButton("Import source"));
						qpb->setEnabled(false);
						foLayout->addWidget(qpb = new QPushButton("Import target"));
						qpb->setEnabled(false);

						layout->addWidget(fileOptions);
					}

					left->addWidget(top);
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