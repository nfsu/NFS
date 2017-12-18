#pragma once

#include <qwidget.h>
#include <NTypes2.h>
#include <FileSystem.h>

class NExplorer;

class MainWindow : public QWidget {

public:

	MainWindow();
	~MainWindow();

	void documentation();
	void findOnline();
	void changeFlag(u32 flag, bool triggered);

private:

	Buffer romData;
	nfs::NDS rom;
	nfs::FileSystem fs;
	NExplorer *nex;

	u32 displayFlag;
};