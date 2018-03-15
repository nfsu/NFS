#include "bitset.h"
#include <math.h>
#include <cstring>
using namespace oi;

boolRef::boolRef(u32 *_ptr, u32 _off): loc(_ptr), offset(_off) { }

boolRef::operator bool() const {
	u32 mask = (u32)1U << offset;
	return (*loc & mask) != 0;
}

boolRef &boolRef::operator=(bool b) {
	u32 mask = (u32)1U << offset;
	(*loc &= (~mask)) |= (mask * (u32)b);
	return *this;
}

bool boolRef::getValue() const {
	u32 mask = (u32)1U << offset;
	return (*loc & mask) != 0;
}

Bitset::~Bitset() {
	if (bits != nullptr) {
		delete[] bits;
		bits = nullptr;
		allocated = stored = 0;
	}
}

Bitset::Bitset(): bits(nullptr), allocated(0), stored(0) {}

Bitset::Bitset(u32 size): Bitset(size, false) {}
Bitset::Bitset(u32 size, bool defVal) : stored(size), allocated((u32)ceil(size / 32.f)) {
	if (size == 0)
		bits = nullptr;
	else {
		bits = new u32[allocated];
		if (!defVal)
			memset(bits, 0, 4U * allocated);
		else
			memset(bits, 255, 4U * allocated);
	}
}

Bitset::Bitset(const Bitset &other) {
	copy(other);
}

Bitset &Bitset::operator=(const Bitset &other) {
	return copy(other);
}

Bitset &Bitset::copy(const Bitset &other) {
	bits = new u32[allocated = other.allocated];
	memcpy(bits, other.bits, sizeof(u32) * allocated);
	stored = other.stored;

	return *this;
}

Bitset::Bitset(Buffer buf, u32 b) {
	allocated = (u32)ceil(b / 64.f);
	bits = new u32[allocated];
	stored = b;
	memset(bits, 0, sizeof(u32) * allocated);
	memcpy(bits, buf.ptr, buf.size);
}

Bitset::operator std::string() { return toString(); }
std::string Bitset::toString() {
	std::string res(stored + 1, '\0');
	for (u32 i = 0; i < stored; ++i)
		res[i] = '0' + (char)getValue(i);

	return res;
}

boolRef Bitset::operator[](u32 at) {
	return boolRef(bits + (at / 32), at % 32);
}

bool Bitset::getValue(u32 at) const {
	return boolRef(bits + (at / 32), at % 32);
}

u32 Bitset::getSize() const { return stored; }

Bitset::operator Buffer() {
	return asBuffer();
}

Buffer Bitset::asBuffer() {
	return { (u32)ceil(stored / 8.f), (u8*)bits };
}

Bitset Bitset::subset(u32 at, u32 length) const {

	Bitset result(length);
	
	for (u32 i = 0; i < length; ++i)
		result[i] = getValue(i + at);

	return result;
}

bool Bitset::operator()(const Bitset &other, u32 start, u32 length, u32 offset) {
	return boolcpy(other, start, length, offset);
}

bool Bitset::boolcpy(const Bitset &other, u32 start, u32 length, u32 offset) {
	for (u32 i = 0; i < length; ++i)
		(*this)[i + offset] = other.getValue(i + start);
	return true;
}

bool Bitset::operator()(const Bitset &other, u32 offset) {
	return boolcpy(other, 0, other.stored, offset);
}

bool Bitset::boolcpy(const Bitset &other, u32 offset) {
	return boolcpy(other, 0, other.stored, offset);
}

bool Bitset::operator==(const Bitset &other) const {
	return stored == other.stored && memcmp(bits, other.bits, allocated * 4U);
}