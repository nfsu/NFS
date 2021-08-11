#pragma once
#include "ntypes.hpp"

namespace nfs {

	using NativeGpuCmdType = u8;

	static inline constexpr u16 GpuCmdType_Make(NativeGpuCmdType opCode, u8 opSize) {
		return (u16(opSize) << 8) | opCode;
	}

	//Can be cast directly to NativeGpuCmdType
	//http://problemkaputt.de/gbatek.htm#ds3dvideo
	enum class GpuCmdType : u16 {

		NOP = 0x00,

		MTX_MODE = 0x10,					//Enable/disable matrices
		MTX_PUSH = 0x11,					//currentMatrix -> stack
		MTX_POP = 0x12,						//stack -> currentMatrix pop
		MTX_SAVE = 0x13,					//currentMatrix -> stack
		MTX_LOAD = 0x14,					//stack[i] -> current
		MTX_IDEN = 0x15,					//Load a unit matrix -> current
		MTX_LOAD_4x4 = 0x16,				//Load 4x4 matrix -> current
		MTX_LOAD_4x3 = 0x17,				//Load 4x3 matrix -> current
		MTX_MUL_4x4 = 0x18,					//Current *= 4x4 matrix
		MTX_MUL_4x3 = 0x19,					//Current *= 4x3 matrix
		MTX_MUL_3x3 = 0x1A,					//Current *= 3x3 matrix
		MTX_SCALE = 0x1B,					//Create a scale matrix f32[3]
		MTX_TRANSLATE = 0x1C,				//Create a translate matrix f32[3]

		LOAD_COLOR = 0x20,					//Load vertex color 16-bit
		LOAD_NORMAL = 0x21,					//Load vertex normal 32-bit
		LOAD_UV = 0x22,						//Load vertex uv 16-bit
		LOAD_XYZ = 0x23,					//Load vertex position 16-bit
		LOAD_XYZ10 = 0x24,					//Load vertex position 10-bit
		LOAD_XY = 0x25,						//Load vpos xy 16-bit
		LOAD_XZ = 0x26,						//Load vpos xz 16-bit
		LOAD_YZ = 0x27,						//Load vpos yz 16-bit
		LOAD_XYZD = 0x28,					//Load vpos from displacement 16-bit

		SET_POLYGON_ATTRIBS = 0x29,			//Set "polygon attributes" (rasterizer settings)
		SET_TEX_ATTRIBS = 0x2A,				//Set "texture params"
		SET_PALETTE = 0x2B,					//Set palette base

		SET_AMBIENT_DIFFUSE = 0x30,			//Set ambient, diffuse, etc.
		SET_SPECULAR_EMISSIVE = 0x31,		//Set specular, emissive, etc.
		SET_LIGHT_VECTOR = 0x32,			//Set directional light direction
		SET_LIGHT_COLOR = 0x33,				//Set light oclor
		SET_SHININESS = 0x34,				//Set shininess or specular

		BEGIN = 0x40,
		END = 0x41,

		//SWAP_BUFFERS = 0x50,
		//VIEWPORT = 0x60,
		//BOX_TEST = 0x70,
		//POS_TEST = 0x71,
		//VEC_TEST = 0x72
	};

	static inline constexpr u8 GpuCmdType_GetSize(GpuCmdType type) { return u8(u16(type) >> 8); }
	static inline constexpr NativeGpuCmdType GpuCmdType_GetOp(GpuCmdType type) { return NativeGpuCmdType(type); }

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

	typedef GpuCmdBase<GpuCmdType::MTX_MODE> GpuCmdModeMtx;
	typedef GpuCmdBase<GpuCmdType::MTX_PUSH> GpuCmdPushMtx;
	typedef GpuCmdBase<GpuCmdType::MTX_POP> GpuCmdPopMtx;
	typedef GpuCmdBase<GpuCmdType::MTX_SAVE> GpuCmdSaveMtx;

	//Load matrix from slot idx as current matrix
	struct GpuCmdMatrix : GpuCmdBase<GpuCmdType::MTX_LOAD> {

		u32 idx;

		void process(GpuCmdParser &parser);
	};

	//Set unit matrix as current matrix
	struct GpuCmdUnitMtx : GpuCmdBase<GpuCmdType::MTX_IDEN> {

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

		union {

			u32 xyz;

			struct {
				u32 z : 10;
				u32 y : 10;
				u32 x : 10;
				u32 _undef : 2;
			};

		};

		void get(fpx1_3_6 &_x, fpx1_3_6 &_y, fpx1_3_6 &_z) {
			_x = fpx1_3_6::dec(x);
			_y = fpx1_3_6::dec(y);
			_z = fpx1_3_6::dec(z);
		}

		void set(fpx1_3_6 _x, fpx1_3_6 _y, fpx1_3_6 _z) {
			x = _x.val;
			y = _y.val;
			z = _z.val;
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

		union {

			u32 xyz;

			struct {
				u32 z : 10;
				u32 y : 10;
				u32 x : 10;
				u32 _undef : 2;
			};

		};

		static constexpr f32 scale = 0.5f * 0.5f * 0.5f;

		void get(f32 &_x, f32 &_y, f32 &_z) {
			_x = fpx1_0_9::dec(x) * scale;
			_y = fpx1_0_9::dec(y) * scale;
			_z = fpx1_0_9::dec(z) * scale;
		}

		void set(f32 _x, f32 _y, f32 _z) {

			fpx1_0_9 i = _x / scale;
			fpx1_0_9 j = _y / scale;
			fpx1_0_9 k = _z / scale;

			x = i.val;
			y = j.val;
			z = k.val;
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

		union {

			u32 xyz;

			struct {
				u32 z : 10;
				u32 y : 10;
				u32 x : 10;
				u32 _undef : 2;
			};

		};

		void get(fpx1_0_9 &_x, fpx1_0_9 &_y, fpx1_0_9 &_z) {
			_x = fpx1_0_9(u16(x));
			_y = fpx1_0_9(u16(y));
			_z = fpx1_0_9(u16(z));
		}

		void set(fpx1_0_9 _x, fpx1_0_9 _y, fpx1_0_9 _z) {
			x = _x.val;
			y = _y.val;
			z = _z.val;
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
	struct GpuCmdScaleMtx : GpuCmdBase<GpuCmdType::MTX_SCALE> {

		fpx1_19_12 x, y, z;

		void process(GpuCmdParser &parser);

	};

	//Translate matrix
	struct GpuCmdTranslateMtx : GpuCmdBase<GpuCmdType::MTX_TRANSLATE> {

		fpx1_19_12 x, y, z;

		void process(GpuCmdParser &parser);

	};

	typedef GpuCmdBase<GpuCmdType::NOP> GpuCmdNop;

	template<typename ...args>
	class TGpuCmds {};

	typedef TGpuCmds<
		GpuCmdNop, GpuCmdModeMtx, GpuCmdPushMtx, GpuCmdPopMtx, GpuCmdSaveMtx, GpuCmdMatrix, GpuCmdUnitMtx,
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

		List<Vertex> vertices;			/* Primitive vertices */

		enum StackType : u32 {

			STACK_PROJECTION,
			STACK_COORDINATE,
			STACK_DIRECTIONAL = 32,
			STACK_TEXTURE = 63,
			STACK_CURRENT,
			STACK_SIZE,

			STACK_COUNT_COORDINATE = STACK_DIRECTIONAL - STACK_COORDINATE,
			STACK_COUNT_DIRECTIONAL = STACK_TEXTURE - STACK_DIRECTIONAL
		};

		f32 matrixStack[STACK_SIZE][4][4];

		void parse(Buffer buffer);

	};

}