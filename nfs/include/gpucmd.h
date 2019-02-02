#pragma once
#include "ntypes.h"

namespace nfs {

	//http://problemkaputt.de/gbatek.htm#ds3dvideo
	enum class GpuCmdType : u8 {
		NOP = 0x00,
		MODE_MTX = 0x10,		/* Enable matrices */
		PUSH_MTX = 0x11,		/* currentMatrix -> stack */
		POP_MTX = 0x12,			/* stack -> currentMatrix pop */
		STORE_MTX = 0x13,		/* currentMatrix -> stack */
		LOAD_MTX = 0x14,		/* stack[i] -> currentMatrix */
		UNIT_MTX = 0x15,		/* Load a unit matrix into current */
		SCALE_MTX = 0x1B,		/* Create a scale matrix f32[3] */
		TRANSLATE_MTX = 0x1C,	/* Create a translate matrix f32[3] */
		LOAD_COLOR = 0x20,		/* Load vertex color 16-bit */
		LOAD_NORMAL = 0x21,		/* Load vertex normal 32-bit? */
		LOAD_UV = 0x22,			/* Load vertex uv 16-bit? */
		LOAD_XYZ = 0x23,		/* Load vertex position 16-bit */
		LOAD_XYZ10 = 0x24,		/* Load vertex position 10-bit */
		LOAD_XY = 0x25,			/* Load vpos xy 16-bit */
		LOAD_XZ = 0x26,			/* Load vpos xz 16-bit */
		LOAD_YZ = 0x27,			/* Load vpos yz 16-bit */
		LOAD_XYZD = 0x28,		/* Load vpos from displacement 16-bit */
		RASTER = 0x29,			/* Set "polygon attributes" (rasterizer settings) */
		BEGIN = 0x40,
		END = 0x41,
	};

	enum class GpuCmdPolyType : u8 {
		TRIANGLES = 0,
		QUADS = 1,
		TRIANGLE_STRIP = 2,
		QUAD_STRIP = 3
	};

	struct GpuCmdParser;

	template<GpuCmdType t>
	struct GpuCmdBase {
		GpuCmdType type = t;
	};

	typedef GpuCmdBase<GpuCmdType::MODE_MTX> GpuCmdModeMtx;
	typedef GpuCmdBase<GpuCmdType::PUSH_MTX> GpuCmdPushMtx;
	typedef GpuCmdBase<GpuCmdType::POP_MTX> GpuCmdPopMtx;
	typedef GpuCmdBase<GpuCmdType::STORE_MTX> GpuCmdStoreMtx;

	//Load matrix from slot idx as current matrix
	struct GpuCmdMatrix : GpuCmdBase<GpuCmdType::LOAD_MTX> {

		u32 idx;

		void process(GpuCmdParser &parser);

	};

	//Set unit matrix as current matrix
	struct GpuCmdUnitMtx : GpuCmdBase<GpuCmdType::UNIT_MTX> {

		void process(GpuCmdParser &parser);

	};

	//Begin 
	struct GpuCmdBegin : GpuCmdBase<GpuCmdType::BEGIN> {

		GpuCmdPolyType type;

		void process(GpuCmdParser &parser);

	};

	//End 
	struct GpuCmdEnd : GpuCmdBase<GpuCmdType::END> {

		void process(GpuCmdParser &parser);

	};

	//Load position
	struct GpuCmdXyz : GpuCmdBase<GpuCmdType::LOAD_XYZ> {

		fpx1_3_12 xyz[3];
		u16 pad;

		void process(GpuCmdParser &parser);

	};

	//Load position
	struct GpuCmdXyz10 : GpuCmdBase<GpuCmdType::LOAD_XYZ10> {

		u32 xyz;

		void get(fpx1_3_6 &x, fpx1_3_6 &y, fpx1_3_6 &z) {
			x = fpx1_3_6::dec((xyz & 0x3FF00000) >> 20);
			y = fpx1_3_6::dec((xyz & 0xFFC00) >> 10);
			z = fpx1_3_6::dec(xyz & 0x3FF);
		}

		void set(fpx1_3_6 x, fpx1_3_6 y, fpx1_3_6 z) {
			xyz = ((u32)x.val << 20) | ((u32)y.val << 10) | z.val;
		}

		void process(GpuCmdParser &parser);

	};

	//Load position
	struct GpuCmdXy : GpuCmdBase<GpuCmdType::LOAD_XY> {

		fpx1_3_12 xy[2];

		void process(GpuCmdParser &parser);

	};

	//Load position
	struct GpuCmdYz : GpuCmdBase<GpuCmdType::LOAD_YZ> {

		fpx1_3_12 yz[2];

		void process(GpuCmdParser &parser);

	};

	//Load position
	struct GpuCmdXz : GpuCmdBase<GpuCmdType::LOAD_XZ> {

		fpx1_3_12 xz[2];

		void process(GpuCmdParser &parser);

	};

	//Load position (displacement)
	struct GpuCmdXyzd : GpuCmdBase<GpuCmdType::LOAD_XYZD> {

		u32 xyz;

		static constexpr f32 scale = 0.5f * 0.5f * 0.5f;

		void get(f32 &x, f32 &y, f32 &z) {
			x = fpx1_0_9::dec((xyz & 0x3FF00000) >> 20) * scale;
			y = fpx1_0_9::dec((xyz & 0xFFC00) >> 10) * scale;
			z = fpx1_0_9::dec(xyz & 0x3FF) * scale;
		}

		void set(f32 _x, f32 _y, f32 _z) {

			fpx1_0_9 x = _x / scale;
			fpx1_0_9 y = _y / scale;
			fpx1_0_9 z = _z / scale;

			xyz = ((u32)x.val << 20) | ((u32)y.val << 10) | z.val;
		}

		void process(GpuCmdParser &parser);

	};

	//Load tex coords
	struct GpuCmdUV : GpuCmdBase<GpuCmdType::LOAD_UV> {

		fpx1_11_4 uv[2];

		void process(GpuCmdParser &parser);

	};

	//Load normal
	struct GpuCmdNormal : GpuCmdBase<GpuCmdType::LOAD_NORMAL> {

		u32 xyz;

		void get(fpx1_0_9 &x, fpx1_0_9 &y, fpx1_0_9 &z) {
			x = fpx1_0_9(u16((xyz & 0x3FF00000) >> 20));
			y = fpx1_0_9(u16((xyz & 0xFFC00) >> 10));
			z = fpx1_0_9(u16(xyz & 0x3FF));
		}

		void set(fpx1_0_9 x, fpx1_0_9 y, fpx1_0_9 z) {
			xyz = ((u32)x.val << 20) | ((u32)y.val << 10) | z.val;
		}

		void process(GpuCmdParser &parser);

	};

	//Load color
	struct GpuCmdColor : GpuCmdBase<GpuCmdType::LOAD_COLOR> {

		col16 rgb;
		u16 pad;

		void process(GpuCmdParser &parser);

	};

	//Scale matrix
	struct GpuCmdScaleMtx : GpuCmdBase<GpuCmdType::SCALE_MTX> {

		fpx1_19_12 x, y, z;

		void process(GpuCmdParser &parser);

	};

	//Translate matrix
	struct GpuCmdTranslateMtx : GpuCmdBase<GpuCmdType::TRANSLATE_MTX> {

		fpx1_19_12 x, y, z;

		void process(GpuCmdParser &parser);

	};

	typedef GpuCmdBase<GpuCmdType::NOP> GpuCmdNop;

	template<typename ...args>
	class TGpuCmds {};

	typedef TGpuCmds<
		GpuCmdNop, GpuCmdModeMtx, GpuCmdPushMtx, GpuCmdPopMtx, GpuCmdStoreMtx, GpuCmdMatrix, GpuCmdUnitMtx,
		GpuCmdScaleMtx, GpuCmdTranslateMtx, GpuCmdXyz, GpuCmdXyz10, GpuCmdXy, GpuCmdXz, GpuCmdYz, GpuCmdXyzd,
		GpuCmdBegin, GpuCmdEnd
	> GpuCmds;

	struct Vertex {
		f32 position[3] = { 0, 0, 0 };
		f32 uv[2] = { 0, 0 };
		f32 normal[3] = { 0, 0, 0 };
		f32 color[3] = { 0, 0, 0 };
	};

	typedef f32 Matrix[4][4];

	struct GpuCmdParser {

		Vertex vertex;					/* Current vertex */

		std::vector<Vertex> vertices;	/* Primitive vertices */

		static constexpr u32 projectionStack = 0,
							coordinateStack = 1,
							directionalStack = 32,
							textureStack = 63,
							currentMatrix = 64;

		f32 matrixStack[
			1 /* Projection */ +
			31 /* Coordinates */ +
			31 /* Directional */ +
			1 /* Texture */ +
			1 /* Current */
		][4][4];

		void parse(Buffer buffer);

	};

}