#include "armulator.h"
using namespace nfs;

#define PRINT_STEP
#define WAIT_STEP

Armulator::Armulator(Buffer buf, u32 entryPoint) : buf(buf) {
	r.pc = entryPoint;
}

CPSR &Armulator::getCPSR() { return cpsr; }
CPSR &Armulator::getSPSR() { return spsr; }
ArmRegisters &Armulator::getRegisters() { return r; }
u8 *Armulator::next() { return buf.ptr + r.pc; }
bool Armulator::thumbMode() { return cpsr.thumbMode; }

void ArmRegisters::printState() {

	printf(
		"r0 = %u\n"
		"r1 = %u\n"
		"r2 = %u\n"
		"r3 = %u\n"
		"r4 = %u\n"
		"r5 = %u\n"
		"r6 = %u\n"
		"r7 = %u\n"
		"r8 = %u\n"
		"r9 = %u\n"
		"r10 = %u\n"
		"r11 = %u\n"
		"r12 = %u\n"
		"r13 = %u\n"
		"r14 = %u\n"
		"r15 = %u\n"
		"pc = %u\n",

		r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9],
		r[10], r[11], r[12], r[13], r[14], pc

	);
}

u32 &ArmRegisters::operator[](size_t i) {
	return d[i];
}

void CPSR::printState() {

	printf(
		"mode = %u\n"
		"thumbMode = %u\n"
		"disableFIQ = %u\n"
		"disableIRQ = %u\n"
		"dataAbortDisable = %u\n"
		"isBigEndian = %u\n"
		"thumbIfThen0 = %u\n"
		"GE = %u\n"
		"nil = %u\n"
		"thumbIfThen1 = %u\n"
		"jazelle = %u\n"
		"overflow = %u\n"
		"carry = %u\n"
		"zero = %u\n"
		"negative = %u\n",
		
		mode, thumbMode, disableFIQ, disableIRQ, dataAbortDisable, isBigEndian,
		thumbIfThen0, GE, nil, thumbIfThen1, jazelle, overflow, carry, zero, negative

	);

}

void Armulator::printState() {

	printf("\nRegisters:\n");
	r.printState();

	printf("\nCPSR:\n");
	cpsr.printState();

	printf("\nSPSR:\n");
	spsr.printState();

}

void Armulator::exec() {
	while (step());
}

bool Armulator::step() {

	if (cpsr.isBigEndian || cpsr.jazelle) {
		EXCEPTION("Big endian and/or Java bytecode are not supported");
		return false;
	}

	bool val = false;

	if (cpsr.thumbMode)
		val = stepThumb();
	else
		val = stepArm();

	#ifdef PRINT_STEP
		printState();
	#endif

	#ifdef WAIT_STEP
		system("pause");
	#endif

	return val;
}

inline bool Armulator::stepThumb() {

	//All of the ways the next instruction can be interpret

	u16 *ptr = (u16*) next();

	ArmThumbOp *op = (ArmThumbOp*) ptr;
	ArmThumbRegOp *regOp = (ArmThumbRegOp*) ptr;

	ArmThumbShift *shift = (ArmThumbShift*) ptr;
	ArmThumbAddSub *addSub = (ArmThumbAddSub*) ptr;
	ArmThumbMovCmpAddSub *movCmpAddSub = (ArmThumbMovCmpAddSub*) ptr;

	//Destination and source

	u32 *Rd = r.d + regOp->Rd, Rs = r[regOp->Rs], mul = 0, val = 0;
	bool negative = false;

	//Execute code

	switch ((ArmThumbOpCodes)op->opCode) {

		//Rd = Rs << offset
		//LSR Rd, Rs, #offset
		case ArmThumbOpCodes::LSL:

			val = *Rd = Rs << shift->offset;
			break;

		//Rd = Rs << offset
		//LSR Rd, Rs, #offset
		case ArmThumbOpCodes::LSR:

			negative = true;
			val = *Rd = Rs >> shift->offset;
			break;

		//Rd = Rs << offset (but keep sign)
		//ASR Rd, Rs, #offset
		case ArmThumbOpCodes::ASR:

			negative = true;
			val = *Rd = ((Rs >> shift->offset) & i32_MAX) | (Rs & i32_MIN);
			break;

		//Rd = Rs + z
		//Rd = Rs - z
		//ADD Rd, Rs, (Rn or #num3bit)
		//SUB Rd, Rs, (Rn or #num3bit)
		case ArmThumbOpCodes::ADD_SUB:

			negative = addSub->sub;
			mul = 1 - addSub->sub * 2;
			val = *Rd = Rs + addSub->Rn * addSub->intermediate * mul + r[addSub->Rn] * (1 - addSub->intermediate) * mul;
			break;

		//Rd = z
		//MOV Rd, #num8bit
		case ArmThumbOpCodes::MOV:

			Rd = r.d + movCmpAddSub->Rd;
			Rs = *Rd;
			val = *Rd = movCmpAddSub->offset;
			break;

		//Rd += z
		//ADD Rd, #num8bit
		case ArmThumbOpCodes::ADD:

			Rd = r.d + movCmpAddSub->Rd;
			Rs = *Rd;
			val = *Rd += movCmpAddSub->offset;
			break;

		//Rd -= z
		//SUB Rd, #num8bit
		case ArmThumbOpCodes::SUB:

			negative = true;
			Rd = r.d + movCmpAddSub->Rd;
			Rs = *Rd;
			val = *Rd -= movCmpAddSub->offset;
			break;

		//Rd -= z
		//SUB Rd, #num8bit
		case ArmThumbOpCodes::CMP:

			negative = true;
			Rd = r.d + movCmpAddSub->Rd;
			Rs = *Rd;
			val = Rs - movCmpAddSub->offset;
			break;

		default:

			EXCEPTION("OpCode was invalid or isn't implemented");
			return false;

	}

	//Set condition flags

	cpsr.zero = val == 0;
	cpsr.negative = (val & i32_MIN) != 0;
	cpsr.carry = (!negative) * (val < Rs);
	cpsr.overflow = (Rs & i32_MIN) != (val & i32_MIN);

	noConditionFlags:

	//Check if there's a next instruction (PC doesn't overflow and doesn't go out of bounds)

	r.pc += 2;
	return r.pc < buf.size && !(r.pc - 2 > r.pc);

}

inline bool Armulator::stepArm() {
	EXCEPTION("Armulator::stepArm not implemented yet");
	return false;
}