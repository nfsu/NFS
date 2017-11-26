#pragma once
#include "NTypes2.h"
#include "API/LM4000_TypeList/TypeStruct.h"

namespace nfs {

	class NHelper {

	public:

		static PaletteTexture2D readNCGR(Buffer buf, u32 paletteOff, u32 textureOff);
		static TiledTexture2D readNCSR(Buffer buf, u32 paletteOff, u32 tilemapOff, u32 mapOff);

		static bool writeNCGR(Buffer buf, u32 paletteOff, u32 textureOff, std::string path);
		static bool writeNCSR(Buffer buf, u32 paletteOff, u32 tilemapOff, u32 mapOff, std::string path);

	private:

		static const bool logTimes;
	};

}