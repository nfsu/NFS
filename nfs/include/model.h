#pragma once

#include "ntypes.h"

namespace nfs {

	struct Info3D {

		struct Header {

			u8 dummy;							//0x0
			u8 objects;
			u16 size;

		} *header;

		struct UnknownBlock {

			struct Header {

				u16 headerSize;						//0x8
				u16 size;

				u32 constant;						//0x17F

			} *header;

			struct Object {

				u16 unknown0;
				u16 unknown1;

			} *objects;	//Info3D::Header.objects

		} block;

		struct InfoBlock {

			struct Header {

				u16 headerSize;
				u16 size;

			} *header;

			u8 *data; //Header.size

		} info;

		struct NameBlock {
			char c[16];
		} *names;

		static Info3D get(u8 *ptr, u32 &len);

	};

	struct ModelData {

		struct Header {

			u32 dataSize;

			u32 renderCmdOffset;

			u32 materialOffset;

			u32 meshOffset;

			u32 meshEnd;		//TODO: Also referenced as "inv_binds_off" (?)

			u8 unk0[3];
			u8 objects;

			u8 materials;
			u8 meshes;
			u8 unk1[2];

			fpx1_19_12 upScale;

			fpx1_19_12 downScale;

			u16 vertices;
			u16 surfaces;

			u16 tris;
			u16 quads;

			fpx1_3_12 minX;
			fpx1_3_12 minY;

			fpx1_3_12 minZ;
			fpx1_3_12 maxX;

			fpx1_3_12 maxY;
			fpx1_3_12 maxZ;

			u8 runtimeData[8];

		} *header;

		struct Mesh {

			struct Header {

				u16 dummy;
				u16 size;

				u32 unk;

				u32 cmdOffset;

				u32 cmdSize;

			} *header;

			u8 *cmd;

		};

		std::vector<Mesh> meshes;

		struct MaterialHeader {

			u16 textureOffset;
			u16 paletteOffset;

		} *materialHeader;

		struct Material {

			enum {
				TextureParameter_Offset,					//u16	: ???
				TextureParameter_RepeatU,					//b1	: If the U coordinate should be repeated
				TextureParameter_RepeatV,					//b1	: If the V coordinate should be repeated
				TextureParameter_MirrorU,					//b1	: If the U coordinate should be mirrored
				TextureParameter_MirrorV,					//b1	: If the V coordinate should be mirrored
				TextureParameter_Width,						//u3	: 8 << width = width
				TextureParameter_Height,					//u3	: 8 << height = height
				TextureParameter_Format,					//u3	: ???
				TextureParameter_NullAsTransparency,		//b1	: Whether or not the 0 value is transparent (true) or black (false)
				TextureParameter_TransformMode				//u2	: ???
			};

			typedef sstruct<u16, b1, b1, b1, b1, u3, u3, u3, b1, u2> TextureParameter;
			
			char name[16];

			u16 dummy;
			u16 section;

			u16 diffuse;
			u16 ambient;

			u16 specular;
			u16 emissive;

			u32 polygonAttrib;

			u32 unk0;

			Material::TextureParameter param;

			u32 unk1;

			u32 unk2;

			u16 width;
			u16 height;

			fpx1_19_12 unk3;

			fpx1_19_12 unk4;

		} *materials;

		struct Object {

			struct Header {

				enum class Transformation : u16 {

					NEGATE_PIVOT = 128,
					NEGATE_SCALE = 64,
					NEGATE_ROTATION = 32,
					NEGATE_TRANSLATE = 16,

					USE_PIVOT = 8,
					SKIP_SCALE = 4,
					SKIP_ROTATION = 2,
					SKIP_TRANSLATE = 1


				} transformation;
				u16 padding;

			} *header;

			struct Translate {
				u32 x, y, z;
			} *translate;				//nullptr if transformation & SKIP_TRANSLATE

			struct Pivot {
				u32 val[2];
			} *pivot;					//nullptr if !(transformation & USE_PIVOT)

			struct Scale {
				u32 x, y, z;
			} *scale;					//nullptr if transformation & SKIP_SCALE

			struct Rotation {
				u32 val[8];
			} *rotation;				//nullptr if transformation & SKIP_ROTATION

		};

		std::vector<Object> objects; //Header.objects

		struct RenderCommand {

			enum class Type : u8 {

				PADDING = 0,
				END = 1,



				BEG_MATERIAL = 0x0B,
				SET_MATERIAL = 0x44,
				END_MATERIAL = 0x2B

			} type;

			u8 *command;

		};

		static ModelData get(u8 *ptr, u32 &len);

	};

	class Model {

	public:

		Model(BMD0 model);

	private:



	};

}