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

	void setup(std::string file);
	void setupRomInfo();

private:

	Buffer romData = { nullptr, 0 };
	nfs::NDS rom;
	nfs::FileSystem fs;
	NExplorer *nex = nullptr;
	std::string fileName = "";
	QLayout *layout = nullptr;

	u32 displayFlag = 0xFFFFFF;
};