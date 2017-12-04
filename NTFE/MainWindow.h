#pragma once

#include <qwidget.h>
#include <NTypes2.h>
#include <FileSystem.h>

class MainWindow : public QWidget {

public:

	MainWindow();
	~MainWindow();

private:

	Buffer romData;
	nfs::NDS rom;
	nfs::FileSystem fs;

};