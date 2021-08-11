#include "bitset.hpp"
#include <math.h>
#include <cstring>
using namespace oi;

boolRef::boolRef(u32 *_ptr, usz _off): loc(_ptr), offset(_off) { }

boolRef::operator bool() const {
	u32 mask = 1 << offset;
	return (*loc & mask) != 0;
}

boolRef &boolRef::operator=(bool b) {
	u32 mask = 1 << offset;
	(*loc &= (~mask)) |= (mask * u32(b));
	return *this;
}

bool boolRef::getValue() const {
	u32 mask = 1 << offset;
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

Bitset::Bitset(usz size): Bitset(size, false) {}
Bitset::Bitset(usz size, bool defVal): 
	stored(size), allocated(usz(ceil(size / 32.0))), bits{}
{
	if (allocated) {
		bits = new u32[allocated];
		::memset(bits, defVal * 0xFF, 4 * allocated);
	}
}

Bitset::Bitset(const Bitset &other) {
	copy(other);
}

Bitset &Bitset::operator=(const Bitset &other) {

	if (bits)
		delete[] bits;

	return copy(other);
}

Bitset &Bitset::copy(const Bitset &other) {

	if (!other.bits) {
		bits = nullptr;
		allocated = stored = 0;
		return *this;
	}

	bits = new u32[allocated = other.allocated];
	std::memcpy(bits, other.bits, sizeof(u32) * allocated);
	stored = other.stored;

	return *this;
}

Bitset::Bitset(Buffer buf, usz b) {

	allocated = usz(ceil(b / 32.0));
	bits = new u32[allocated];
	stored = b;

	std::memset(bits, 0, sizeof(u32) * allocated);
	std::memcpy(bits, buf.add(), buf.size());
}

Bitset::operator String() { return toString(); }

String Bitset::toString() {

	String res(stored + 1, '\0');

	static constexpr c8 opts[2] = { '0', '1' };

	for (usz i = 0; i < stored; ++i)
		res[i] = opts[getValue(i)];

	return res;
}

boolRef Bitset::operator[](usz at) {
	return boolRef(bits + (at >> 5), at & 31);
}

bool Bitset::getValue(usz at) const {
	return boolRef(bits + (at >> 5), at & 31);
}

usz Bitset::getSize() const { return stored; }

Bitset::operator Buffer() {
	return asBuffer();
}

Buffer Bitset::asBuffer() {
	return { usz(ceil(stored / 8.0)), (u8*) bits };
}

Bitset Bitset::subset(usz at, usz length) const {

	Bitset result(length);
	
	for (usz i = 0; i < length; ++i)
		result[i] = getValue(i + at);

	return result;
}

bool Bitset::operator()(const Bitset &other, usz start, usz length, usz offset) {
	return boolcpy(other, start, length, offset);
}

bool Bitset::boolcpy(const Bitset &other, usz start, usz length, usz offset) {

	for (usz i = 0; i < length; ++i)
		(*this)[i + offset] = other.getValue(i + start);

	return true;
}

bool Bitset::operator()(const Bitset &other, usz offset) {
	return boolcpy(other, 0, other.stored, offset);
}

bool Bitset::boolcpy(const Bitset &other, usz offset) {
	return boolcpy(other, 0, other.stored, offset);
}

bool Bitset::operator==(const Bitset &other) const {
	return stored == other.stored && std::memcmp(bits, other.bits, allocated * 4);
}