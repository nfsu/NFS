#include "game_editor.hpp"
#include "tile_editor.hpp"
#include "info_window.hpp"
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

void GameEditor::init(NDS *nds, FileSystem&) {
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

enum GameEditorType : u32 {

	TITLE,
	GAME_CODE,
	MAKER_CODE,
	UNIT_CODE,
	VERSION,
	ROM_SIZE,
	HEADER_SIZE,
	DEBUG_OFFSET,
	DEBUG_SIZE,
	FILE_TABLE_OFFSET,
	FILE_TABLE_SIZE,
	FILE_ALLOCATION_OFFSET,
	FILE_ALLOCATION_SIZE,

	ARM7_OFFSET,
	ARM7_SIZE,
	ARM7_ENTRY,
	ARM7_LOAD,
	ARM7_ALLRA,
	ARM7_OVERLAY_OFFSET,
	ARM7_OVERLAY_SIZE,

	ARM9_OFFSET,
	ARM9_SIZE,
	ARM9_ENTRY,
	ARM9_LOAD,
	ARM9_ALLRA,
	ARM9_OVERLAY_OFFSET,
	ARM9_OVERLAY_SIZE,

	COUNT,
	BEGIN = TITLE
};

static const c8 *strings[] = {
	"Title",
	"Game code",
	"Maker code",
	"Unit code",
	"Version",
	"Size",
	"Header size",
	"Debug offset",
	"Debug size",
	"File table offset",
	"File table size",
	"File allocation offset",
	"File allocation size",
	"ARM7 offset",
	"ARM7 size",
	"ARM7 entry",
	"ARM7 load",
	"ARM7 ALLRA",
	"ARM7 overlay offset",
	"ARM7 overlay size",
	"ARM9 offset",
	"ARM9 size",
	"ARM9 entry",
	"ARM9 load",
	"ARM9 ALLRA",
	"ARM9 overlay offset",
	"ARM9 overlay size"
};

inline void set(InfoWindow *info, GameEditorType type, const QString &str) {
	info->setString(strings[type], str);
}

inline void set(InfoWindow *info, GameEditorType type, const String &str) {
	info->setString(strings[type], QString::fromStdString(str));
}

void GameEditor::showInfo(InfoWindow *info) {

	if (rom != nullptr) {

		String title = String(rom->title, rom->title + sizeof(rom->title));
		title = title.substr(0, title.find_last_of('\0'));

		String gameCode = String(rom->gameCode, rom->gameCode + sizeof(rom->gameCode));
		gameCode = gameCode.substr(0, gameCode.find_last_of('\0'));

		String makerCode = String(rom->makerCode, rom->makerCode + sizeof(rom->makerCode));
		makerCode = makerCode.substr(0, makerCode.find_last_of('\0'));

		set(info, TITLE, title);
		set(info, GAME_CODE, gameCode);
		set(info, MAKER_CODE, makerCode);
		set(info, UNIT_CODE, QString::number(rom->unitCode));
		set(info, VERSION, QString::number(rom->version));
		set(info, ROM_SIZE, QString("#") + QString::number(rom->romSize, 16));
		set(info, HEADER_SIZE, QString("#") + QString::number(rom->romHeaderSize, 16));
		set(info, DEBUG_OFFSET, QString("#") + QString::number(rom->dRomOff, 16));
		set(info, DEBUG_SIZE, QString("#") + QString::number(rom->dRomSize, 16));
		set(info, FILE_TABLE_OFFSET, QString("#") + QString::number(rom->ftable_off, 16));
		set(info, FILE_TABLE_SIZE, QString("#") + QString::number(rom->ftable_len, 16));
		set(info, FILE_ALLOCATION_OFFSET, QString("#") + QString::number(rom->falloc_off, 16));
		set(info, FILE_ALLOCATION_SIZE, QString("#") + QString::number(rom->falloc_len, 16));

		//TODO: Show as buffer editor

		set(info, ARM7_OFFSET, QString("#") + QString::number(rom->arm7_offset, 16));
		set(info, ARM7_SIZE, QString("#") + QString::number(rom->arm7_size, 16));
		set(info, ARM7_ENTRY, QString("#") + QString::number(rom->arm7_entry, 16));
		set(info, ARM7_LOAD, QString("#") + QString::number(rom->arm7_load, 16));
		set(info, ARM7_ALLRA, QString("#") + QString::number(rom->arm7_ALLRA, 16));
		set(info, ARM7_OVERLAY_OFFSET, QString("#") + QString::number(rom->arm7_ooff, 16));
		set(info, ARM7_OVERLAY_SIZE, QString("#") + QString::number(rom->arm7_olen, 16));
		set(info, ARM9_OFFSET, QString("#") + QString::number(rom->arm9_offset, 16));
		set(info, ARM9_SIZE, QString("#") + QString::number(rom->arm9_size, 16));
		set(info, ARM9_ENTRY, QString("#") + QString::number(rom->arm9_entry, 16));
		set(info, ARM9_LOAD, QString("#") + QString::number(rom->arm9_load, 16));
		set(info, ARM9_ALLRA, QString("#") + QString::number(rom->arm9_ALLRA, 16));
		set(info, ARM9_OVERLAY_OFFSET, QString("#") + QString::number(rom->arm9_ooff, 16));
		set(info, ARM9_OVERLAY_SIZE, QString("#") + QString::number(rom->arm9_olen, 16));
	}

	else for (u32 i = BEGIN; i != COUNT; ++i)
		info->setString(strings[i], "");
}

void GameEditor::hideInfo(InfoWindow *info) {

	for (u32 i = BEGIN; i != COUNT; ++i)
		info->clearString(strings[i]);
}