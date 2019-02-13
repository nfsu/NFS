#include "gameeditor.h"
#include "tileeditor.h"
#include "infowindow.h"
using namespace nfsu;
using namespace nfs;

GameEditor::GameEditor() {
	addWidget(logoEditor = new TileEditor);
}

TileEditor *GameEditor::getLogoEditor() {
	return logoEditor;
}

void GameEditor::onSwap() {

	if (rom == nullptr)
		return;

	logoEditor->setPalette(Texture2D((u8*)bannerData->palette, 16, 1, 2, TextureType::BGR5));
	logoEditor->setTiles(Texture2D((u8*)bannerData->icon, 32, 32, 1, TextureType::R4, TextureTiles::TILED8));

}

void GameEditor::init(NDS *nds, FileSystem &fs) {
	rom = nds;
	bannerData = NDSBanner::get(nds);
	onSwap();
}

void GameEditor::reset() {
	rom = nullptr;
	bannerData = nullptr;
	logoEditor->setPalette(Texture2D());
	logoEditor->setTiles(Texture2D());
}

void GameEditor::showInfo(InfoWindow *info) {

	if (rom != nullptr) {

		info->setString("Title", rom->title);
		info->setString("Game code", rom->gameCode);
		info->setString("Maker code", rom->makerCode);
		info->setString("Unit code", QString::number(rom->unitCode));
		info->setString("Version", QString::number(rom->version));
		info->setString("Size", QString("#") + QString::number(rom->romSize, 16));
		info->setString("Header size", QString("#") + QString::number(rom->romHeaderSize, 16));
		info->setString("Debug offset", QString("#") + QString::number(rom->dRomOff, 16));
		info->setString("Debug size", QString("#") + QString::number(rom->dRomSize, 16));
		info->setString("File table offset", QString("#") + QString::number(rom->ftable_off, 16));
		info->setString("File table size", QString("#") + QString::number(rom->ftable_len, 16));
		info->setString("File allocation offset", QString("#") + QString::number(rom->falloc_off, 16));
		info->setString("File allocation size", QString("#") + QString::number(rom->falloc_len, 16));

		//TODO: Show as buffer editor

		info->setString("ARM7 offset", QString("#") + QString::number(rom->arm7_offset, 16));
		info->setString("ARM7 size", QString("#") + QString::number(rom->arm7_size, 16));
		info->setString("ARM7 entry", QString("#") + QString::number(rom->arm7_entry, 16));
		info->setString("ARM7 load", QString("#") + QString::number(rom->arm7_load, 16));
		info->setString("ARM7 ALLRA", QString("#") + QString::number(rom->arm7_ALLRA, 16));
		info->setString("ARM7 overlay offset", QString("#") + QString::number(rom->arm7_ooff, 16));
		info->setString("ARM7 overlay size", QString("#") + QString::number(rom->arm7_olen, 16));
		info->setString("ARM9 offset", QString("#") + QString::number(rom->arm9_offset, 16));
		info->setString("ARM9 size", QString("#") + QString::number(rom->arm9_size, 16));
		info->setString("ARM9 entry", QString("#") + QString::number(rom->arm9_entry, 16));
		info->setString("ARM9 load", QString("#") + QString::number(rom->arm9_load, 16));
		info->setString("ARM9 ALLRA", QString("#") + QString::number(rom->arm9_ALLRA, 16));
		info->setString("ARM9 overlay offset", QString("#") + QString::number(rom->arm9_ooff, 16));
		info->setString("ARM9 overlay size", QString("#") + QString::number(rom->arm9_olen, 16));

	} else {
	
		info->setString("Title", "");
		info->setString("Game code", "");
		info->setString("Maker code", "");
		info->setString("Unit code", "");
		info->setString("Version", "");
		info->setString("Size", "");
		info->setString("Header size", "");
		info->setString("Debug offset", "");
		info->setString("Debug size", "");
		info->setString("File table offset", "");
		info->setString("File table size", "");
		info->setString("File allocation offset", "");
		info->setString("File allocation size", "");

		info->setString("ARM7 offset", "");
		info->setString("ARM7 size", "");
		info->setString("ARM7 entry", "");
		info->setString("ARM7 load", "");
		info->setString("ARM7 ALLRA", "");
		info->setString("ARM7 overlay offset", "");
		info->setString("ARM7 overlay size", "");
		info->setString("ARM9 offset", "");
		info->setString("ARM9 size", "");
		info->setString("ARM9 entry", "");
		info->setString("ARM9 load", "");
		info->setString("ARM9 ALLRA", "");
		info->setString("ARM9 overlay offset", "");
		info->setString("ARM9 overlay size", "");

	}

}

void GameEditor::hideInfo(InfoWindow *info) {

	info->clearString("Title");
	info->clearString("Game code");
	info->clearString("Maker code");
	info->clearString("Unit code");
	info->clearString("Version");
	info->clearString("Size");
	info->clearString("Header size");
	info->clearString("Debug offset");
	info->clearString("Debug size");
	info->clearString("File table offset");
	info->clearString("File table size");
	info->clearString("File allocation offset");
	info->clearString("File allocation size");

	info->clearString("ARM7 offset");
	info->clearString("ARM7 size");
	info->clearString("ARM7 entry");
	info->clearString("ARM7 load");
	info->clearString("ARM7 ALLRA");
	info->clearString("ARM7 overlay offset");
	info->clearString("ARM7 overlay size");
	info->clearString("ARM9 offset");
	info->clearString("ARM9 size");
	info->clearString("ARM9 entry");
	info->clearString("ARM9 load");
	info->clearString("ARM9 ALLRA");
	info->clearString("ARM9 overlay offset");
	info->clearString("ARM9 overlay size");

}