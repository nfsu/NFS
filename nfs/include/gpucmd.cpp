#include "gpucmd.h"
using namespace nfs;

template<typename T, typename ...args>
struct _ProcessGpuCmd {

	static void process(Buffer &buffer, u8 dat) {

		if (T{}.type == dat) {
			((T*)buffer.ptr)->process();
			buffer.addOffset(u32(sizeof(T)));
		} else
			_ProcessGpuCmd<args...>::process(buffer, dat);

	}

};

template<typename T>
struct _ProcessGpuCmd {

	static void process(Buffer &buffer, u8 dat) {

		if (T{}.type == dat) {
			((T*)buffer.ptr)->process();
			buffer.addOffset(u32(sizeof(T)));
		} else
			throw std::runtime_error("Couldn't find GPU command; it's not implemented");

	}

};

struct ProcessGpuCmd {

	template<typename ...args>
	static void process(Buffer &buffer, u8 dat, TGpuCmds<args...>) {
		_ProcessGpuCmd<args...>::process(buffer, dat);
	}

	static void process(Buffer &buffer) {
		ProcessGpuCmd::process(buffer, buffer[0], GpuCmds{});
	}

};

const Matrix identity = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };

void GpuCmdUnitMtx::process(GpuCmdParser &parser) {
	memcpy(parser.matrixStack[GpuCmdParser::currentMatrix], identity, sizeof(identity));
}

void GpuCmdScaleMtx::process(GpuCmdParser &parser) {
	const Matrix scale = { { x, 0, 0, 0 }, { 0, y, 0, 0 }, { 0, 0, z, 0 }, { 0, 0, 0, 1 } };
	memcpy(parser.matrixStack[GpuCmdParser::currentMatrix], scale, sizeof(scale));
}

void GpuCmdTranslateMtx::process(GpuCmdParser &parser) {
	const Matrix scale = { { 0, 0, 0, x }, { 0, 0, 0, y }, { 0, 0, 0, z }, { 0, 0, 0, 1 } };
	memcpy(parser.matrixStack[GpuCmdParser::currentMatrix], scale, sizeof(scale));
}

void GpuCmdParser::parse(Buffer buffer) {

	while (buffer.size > 0) {
		ProcessGpuCmd::process(buffer);
	}

}