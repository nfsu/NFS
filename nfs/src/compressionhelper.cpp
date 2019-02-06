#include "compressionhelper.h"
using namespace nfs;

f32 CompressionHelper::normalize(u32 val, u32 maxVal) {
	return f32(val) / maxVal;
}

u8 CompressionHelper::fromBGR5(u8 val) {
	return u8(val / 31.f * 255);
}

u32 CompressionHelper::samplePixel(u16 pix) {

	u32 b = fromBGR5(pix & 0x1FU);
	u32 g = fromBGR5((pix & 0x3E0U) >> 5U);
	u32 r = fromBGR5((pix & 0x7C00U) >> 10U);

	return (0xFFU << 24U) | (r << 16U) | (g << 8U) | b;

}

std::array<f32, 3> CompressionHelper::sampleColor(u16 pix) {

	f32 b = normalize(pix & 0x1FU, 0x1FU);
	f32 g = normalize((pix & 0x3E0U) >> 5U, 0x1FU);
	f32 r = normalize((pix & 0x7C00U) >> 10U, 0x1FU);

	return { r, g, b };
}

u32 CompressionHelper::unnormalize(f32 val, u32 maxVal) {
	return u32(val * maxVal);
}

u8 CompressionHelper::toBGR5(u8 val) {
	return u8(val / 255.f * 31);
}

u16 CompressionHelper::storePixel(u32 pix) {

	u32 r = toBGR5(pix & 0xFFU);
	u32 g = toBGR5((pix & 0xFF00U) >> 8U);
	u32 b = toBGR5((pix & 0xFF0000U) >> 16U);

	return b | (g << 5U) | (r << 10U);

}

u16 CompressionHelper::storeColor(std::array<f32, 3> val) {

	u16 r = u16(unnormalize(val[0], 0x1FU));
	u16 g = u16(unnormalize(val[1], 0x1FU));
	u16 b = u16(unnormalize(val[2], 0x1FU));

	return b | (g << 5U) | (r << 10U);

}

u32 CompressionHelper::generateRandom(u32 seed, const u32 multiply, const u32 add) {
	return multiply * seed + add;
}