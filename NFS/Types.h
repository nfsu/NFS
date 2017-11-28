#pragma once

#include "Generic.h"

///Structs

//BitDepth of a palette;
//BD_NONE for regular palettes, BD_FOUR for palettes with 16 values and BD_EIGHT for palettes with 256 values
typedef enum {
	BD_NONE, BD_FOUR = 3, BD_EIGHT = 4
} BitDepth;