#pragma once
#include "generic.h"

namespace nfs {

	struct CompressionHelper {

		//Get a 16-bit color to linear color space
		static std::array<f32, 3> sampleColor(u16 val);

		//Get a 16-bit color as a 32-bit color (full opacity; alpha 0xFF)
		static u32 samplePixel(u16 val);

		//Get a 32-bit color as a 16-bit color
		static u16 storePixel(u32 val);

		//Get a linear color as 16-bit
		static u16 storeColor(std::array<f32, 3> val);

		//Normalize an int in range [0, maxVal]
		static f32 normalize(u32 val, u32 maxVal);

		//Get a 5-bit channel as 8-bit
		static u8 fromBGR5(u8 val);

		//Unnormalize float range [0,1] to [0,maxVal]
		static u32 unnormalize(f32 val, u32 maxVal);

		//Get a 8-bit channel as 5-bit
		static u8 toBGR5(u8 val);

	};

}