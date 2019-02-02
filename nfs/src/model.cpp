#include "model.h"
using namespace nfs;

Info3D Info3D::get(u8 *ptr, u32 &len) {

	Info3D inf;

	inf.header = (Header*)ptr;
	ptr += sizeof(Header);

	inf.block.header = (UnknownBlock::Header*)ptr;
	ptr += sizeof(UnknownBlock::Header);

	inf.block.objects = (UnknownBlock::Object*)ptr;
	ptr += sizeof(UnknownBlock::Object) * inf.header->objects;

	inf.info.header = (InfoBlock::Header*)ptr;
	ptr += sizeof(InfoBlock::Header);

	inf.info.data = ptr;
	ptr += inf.info.header->size;

	inf.names = (NameBlock*)ptr;
	ptr += inf.info.header->size;

	len = inf.header->size;

	return inf;
}

ModelData ModelData::get(u8 *ptr, u32 &len) {

	ModelData dat;

	dat.header = (Header*)ptr;

	f32 upScale = dat.header->upScale;
	f32 downScale = dat.header->downScale;

	//TODO: use upScale, downScale

	f32 aabbx = dat.header->minX;
	f32 aabby = dat.header->minY;
	f32 aabbz = dat.header->minZ;
	f32 aabbw = dat.header->maxX;
	f32 aabbh = dat.header->maxY;
	f32 aabbl = dat.header->maxZ;

	//Parse materials

	u8 *mat = ptr + dat.header->materialOffset - 40 + 0x40;

	std::vector<u8> test(mat, mat + 256);

	dat.materialHeader = (MaterialHeader*)mat;
	mat += sizeof(MaterialHeader);
	dat.materials = (Material*)mat;

	int dbg = 0;

	//Buffer renderCommands = Buffer(dat.header->materialOffset - dat.header->renderCmdOffset, ptr + dat.header->renderCmdOffset - len);

	/*if (dat.header->meshes != 0) {

		dat.meshes.resize(dat.header->meshes);

		u8 *mdlOffset = ptr + dat.header->meshOffset;

		for (u32 i = 0; i < dat.header->meshes; ++i) {

			Mesh &m = dat.meshes[i];

			m.header = (Mesh::Header*) mdlOffset;
			mdlOffset += sizeof(Mesh::Header);

			m.cmd = ptr + m.header->cmdOffset - len;

		}

	}*/



	/*for (u32 i = 0; i < dat.header->objects; ++i) {

	}*/

	return dat;

}

Model::Model(BMD0 bmd0) {

	MDL0 mdl0 = bmd0.at<0>();
	TEX0 tex0 = bmd0.at<1>();
	Buffer mdl0b = bmd0.get<0>();
	Buffer tex0b = bmd0.get<1>();

	u32 i = 0;
	Info3D info = Info3D::get(mdl0b.ptr, i);
	ModelData data = ModelData::get(mdl0b.ptr + i, i);

}