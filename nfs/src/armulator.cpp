#include "armulator.h"
using namespace nfs;

#define PRINT_STEP
#define WAIT_STEP
#define PRINT_INSTRUCTION

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
		"mode = %u\t\t\t"
		"thumbMode = %u\t\t\t"
		"disableFIQ = %u\t\t"
		"disableIRQ = %u\n"
		"dataAbortDisable = %u\t\t"
		"isBigEndian = %u\t\t\t"
		"thumbIfThen0 = %u\t"
		"GE = %u\n"
		"nil = %u\t\t\t\t"
		"thumbIfThen1 = %u\t\t"
		"jazelle = %u\t\t"
		"overflow = %u\n"
		"carry = %u\t\t\t"
		"zero = %u\t\t\t"
		"negative = %u\n",
		
		mode, thumbMode, disableFIQ, disableIRQ, dataAbortDisable, isBigEndian,
		thumbIfThen0, GE, nil, thumbIfThen1, jazelle, overflow, carry, zero, negative

	);

}

void Armulator::printState() {

	printf("\nCPSR:\n");
	cpsr.printState();

	printf("\nRegisters:\n");
	r.printState();

	if (cpsr.mode != (u32) CPSR::Mode::USR) {
		printf("\nSPSR:\n");
		spsr.printState();
	}

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

	#ifdef PRINT_STEP
		printf("\n\n\n\n");
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

			#ifdef PRINT_INSTRUCTION
				printf("LSL r%u, r%u, #%p\n", regOp->Rd, regOp->Rs, (void*) shift->offset);
			#endif

			val = *Rd = Rs << shift->offset;
			break;

		//Rd = Rs << offset
		//LSR Rd, Rs, #offset
		case ArmThumbOpCodes::LSR:

			#ifdef PRINT_INSTRUCTION
				printf("LSR r%u, r%u, #%p\n", regOp->Rd, regOp->Rs, (void*) shift->offset);
			#endif

			negative = true;
			val = *Rd = Rs >> shift->offset;
			break;

		//Rd = Rs << offset (but keep sign)
		//ASR Rd, Rs, #offset
		case ArmThumbOpCodes::ASR:

			#ifdef PRINT_INSTRUCTION
				printf("ASR r%u, r%u, #%p\n", regOp->Rd, regOp->Rs, (void*) shift->offset);
			#endif

			negative = true;
			val = *Rd = ((Rs >> shift->offset) & i32_MAX) | (Rs & i32_MIN);
			break;

		//Rd = Rs + z
		//Rd = Rs - z
		//ADD Rd, Rs, (Rn or #num3bit)
		//SUB Rd, Rs, (Rn or #num3bit)
		case ArmThumbOpCodes::ADD_SUB:

			#ifdef PRINT_INSTRUCTION
				if(addSub->sub)
					if(addSub->intermediate)
						printf("SUB r%u, r%u, #%p\n", regOp->Rd, regOp->Rs, (void*)addSub->Rn);
					else
						printf("SUB r%u, r%u, r%u\n", regOp->Rd, regOp->Rs, addSub->Rn);
				else {
					if (addSub->intermediate)
						printf("ADD r%u, r%u, #%p\n", regOp->Rd, regOp->Rs, (void*)addSub->Rn);
					else
						printf("ADD r%u, r%u, r%u\n", regOp->Rd, regOp->Rs, addSub->Rn);
				}
			#endif

			negative = addSub->sub;
			mul = 1 - addSub->sub * 2;
			val = *Rd = Rs + addSub->Rn * addSub->intermediate * mul + r[addSub->Rn] * (1 - addSub->intermediate) * mul;
			break;

		//Rd = z
		//MOV Rd, #num8bit
		case ArmThumbOpCodes::MOV:

			#ifdef PRINT_INSTRUCTION
				printf("MOV r%u, #%p\n", movCmpAddSub->Rd, (void*) movCmpAddSub->offset);
			#endif

			Rd = r.d + movCmpAddSub->Rd;
			Rs = *Rd;
			val = *Rd = movCmpAddSub->offset;
			break;

		//Rd += z
		//ADD Rd, #num8bit
		case ArmThumbOpCodes::ADD:

			#ifdef PRINT_INSTRUCTION
				printf("ADD r%u, #%p\n", movCmpAddSub->Rd, (void*) movCmpAddSub->offset);
			#endif

			Rd = r.d + movCmpAddSub->Rd;
			Rs = *Rd;
			val = *Rd += movCmpAddSub->offset;
			break;

		//Rd -= z
		//SUB Rd, #num8bit
		case ArmThumbOpCodes::SUB:

			#ifdef PRINT_INSTRUCTION
				printf("SUB r%u, #%p\n", movCmpAddSub->Rd, (void*) movCmpAddSub->offset);
			#endif

			negative = true;
			Rd = r.d + movCmpAddSub->Rd;
			Rs = *Rd;
			val = *Rd -= movCmpAddSub->offset;
			break;

		//Rd - z
		//CMP Rd, #num8bit
		case ArmThumbOpCodes::CMP:

			#ifdef PRINT_INSTRUCTION
				printf("CMP r%u, #%p\n", movCmpAddSub->Rd, (void*) movCmpAddSub->offset);
			#endif

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