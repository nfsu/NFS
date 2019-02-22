#include "armulator.h"
using namespace nfs;
using namespace arm;
using namespace thumb;

#define PRINT_STEP
#define WAIT_STEP
#define PRINT_INSTRUCTION

Armulator::Armulator(Buffer buf, u32 entryPoint) : buf(buf) {
	r.pc = entryPoint;
}

u32 *RegisterBank::find(size_t i) {
	u8 reg = RegisterBank::registerMapping[CPSR::modeId[cpsr.mode]][i];
	return dat + reg;
}

u32 &RegisterBank::operator[](size_t i) {
	return *find(i);
}

u32 &RegisterBank::get(size_t i) {
	return *find(i);
}

void RegisterBank::printState() {

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
		"sp = %u\n"
		"lr = %u\n"
		"pc = %u\n",

		get(0), get(1), get(2), get(3), get(4), get(5), get(6), get(7), get(8), get(9),
		get(10), get(11), get(12), get(13), get(14), pc

	);
}

void CPSR::printState() {

	printf(
		"mode = %u\t\t\t"
		"thumbMode = %u\t\t\t"
		"disableFIQ = %u\t\t"
		"disableIRQ = %u\n"
		"overflow = %u\t\t\t"
		"carry = %u\t\t\t"
		"zero = %u\t\t"
		"negative = %u\n",
		
		mode, thumbMode, disableFIQ, disableIRQ, overflow, carry, zero, negative

	);

}

void Armulator::printState() {

	printf("\nCPSR:\n");
	r.cpsr.printState();

	printf("\nRegisters:\n");
	r.printState();

	u8 cpsrIdx = CPSR::modeId[r.cpsr.mode];

	if (cpsrIdx != 0) {
		printf("\nSPSR:\n");
		r.spsr[cpsrIdx - 1].printState();
	}

}

void Armulator::exec() {
	while (doStep());
}

bool Armulator::step() {
	return doStep();
}

bool Armulator::doStep() {

	bool val = false;

	if (r.cpsr.thumbMode)
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

bool Armulator::condition(Condition::Value condition) {
	return doCondition(condition);
}

inline bool Armulator::doCondition(Condition::Value condition) {

	bool val;

	switch (condition) {

	case Condition::EQ:
	case Condition::NE:
		val = r.cpsr.zero;
		break;

	case Condition::CS:
	case Condition::CC:
		val = r.cpsr.carry;
		break;

	case Condition::MI:
	case Condition::PL:
		val = r.cpsr.negative;
		break;

	case Condition::VS:
	case Condition::VC:
		val = r.cpsr.overflow;
		break;

	case Condition::HI:
	case Condition::LS:
		val = r.cpsr.carry && !r.cpsr.zero;
		break;

	case Condition::GE:
	case Condition::LT:
		val = r.cpsr.negative == r.cpsr.overflow;
		break;

	case Condition::GT:
	case Condition::LE:
		val = r.cpsr.zero && r.cpsr.negative == r.cpsr.overflow;
		break;

	default:
		val = true;

	}

	return val != ((u32)condition & 1);
}

void Armulator::IRQ() {

	if (r.cpsr.disableIRQ)
		return;

	switchMode(CPSR::Mode::IRQ);
	r.cpsr.disableIRQ = 1;

}

void Armulator::FIQ() {

	if (r.cpsr.disableFIQ)
		return;

	switchMode(CPSR::Mode::FIQ);
	r.cpsr.disableFIQ = 1;

}

void Armulator::switchMode(CPSR::Mode mode) {

	u8 idx = CPSR::modeId[(u32) mode];

	r[14] = r.pc;

	if (idx != 0)
		r.spsr[idx - 1] = r.cpsr;

	switch (mode) {

	case CPSR::Mode::USR:
	case CPSR::Mode::FIQ:
	case CPSR::Mode::IRQ:
	case CPSR::Mode::SVC:
	case CPSR::Mode::ABT:
	case CPSR::Mode::UND:
	case CPSR::Mode::SYS:
		break;

	default:

		EXCEPTION("Switching to undefined assembly mode");
		mode = CPSR::Mode::USR;

	}

	r.cpsr.mode = (u32) mode;

}

inline bool Armulator::stepThumb() {

	//All of the ways the next instruction can be interpret

	u32 pc = r.pc & ~1;
	u16 *ptr = (u16*)(buf.ptr + pc);

	Op *op = (Op*) ptr;
	RegOp *regOp = (RegOp*) ptr;

	Shift *shift = (Shift*) ptr;
	AddSub *addSub = (AddSub*) ptr;
	MovCmpAddSub *movCmpAddSub = (MovCmpAddSub*) ptr;

	CondBranch *condBranch = (CondBranch*) ptr;
	Branch *branch = (Branch*) ptr;

	//Destination and source

	u32 *Rd = r.find(regOp->Rd),
		Rs = r[regOp->Rs], 
		mul = 0, val = 0;

	bool negative = false;

	//Execute code

	switch ((OpCode) op->code) {

		//Rd = Rs << offset
		//LSR Rd, Rs, #offset
		case OpCode::LSL:

			#ifdef PRINT_INSTRUCTION
				printf("LSL r%u, r%u, #%u\n", regOp->Rd, regOp->Rs, shift->offset);
			#endif

			val = *Rd = Rs << shift->offset;
			break;

		//Rd = Rs << offset
		//LSR Rd, Rs, #offset
		case OpCode::LSR:

			#ifdef PRINT_INSTRUCTION
				printf("LSR r%u, r%u, #%u\n", regOp->Rd, regOp->Rs, shift->offset);
			#endif

			negative = true;
			val = *Rd = Rs >> shift->offset;
			break;

		//Rd = Rs << offset (but keep sign)
		//ASR Rd, Rs, #offset
		case OpCode::ASR:

			#ifdef PRINT_INSTRUCTION
				printf("ASR r%u, r%u, #%u\n", regOp->Rd, regOp->Rs, shift->offset);
			#endif

			negative = true;
			val = *Rd = ((Rs >> shift->offset) & i32_MAX) | (Rs & i32_MIN);
			break;

		//Rd = Rs + z
		//Rd = Rs - z
		//ADD Rd, Rs, (Rn or #num3bit)
		//SUB Rd, Rs, (Rn or #num3bit)
		case OpCode::ADD_SUB:

			#ifdef PRINT_INSTRUCTION
				if(addSub->sub)
					if(addSub->intermediate)
						printf("SUB r%u, r%u, #%u\n", regOp->Rd, regOp->Rs, addSub->Rn);
					else
						printf("SUB r%u, r%u, r%u\n", regOp->Rd, regOp->Rs, addSub->Rn);
				else {
					if (addSub->intermediate)
						printf("ADD r%u, r%u, #%u\n", regOp->Rd, regOp->Rs, addSub->Rn);
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
		case OpCode::MOV:

			#ifdef PRINT_INSTRUCTION
				printf("MOV r%u, #%u\n", movCmpAddSub->Rd, movCmpAddSub->offset);
			#endif

			Rd = r.find(movCmpAddSub->Rd);
			Rs = *Rd;
			val = *Rd = movCmpAddSub->offset;
			break;

		//Rd += z
		//ADD Rd, #num8bit
		case OpCode::ADD:

			#ifdef PRINT_INSTRUCTION
				printf("ADD r%u, #%u\n", movCmpAddSub->Rd, movCmpAddSub->offset);
			#endif

			Rd = r.find(movCmpAddSub->Rd);
			Rs = *Rd;
			val = *Rd += movCmpAddSub->offset;
			break;

		//Rd -= z
		//SUB Rd, #num8bit
		case OpCode::SUB:

			#ifdef PRINT_INSTRUCTION
				printf("SUB r%u, #%u\n", movCmpAddSub->Rd, movCmpAddSub->offset);
			#endif

			negative = true;
			Rd = r.find(movCmpAddSub->Rd);
			Rs = *Rd;
			val = *Rd -= movCmpAddSub->offset;
			break;

		//Rd - z
		//CMP Rd, #num8bit
		case OpCode::CMP:

			#ifdef PRINT_INSTRUCTION
				printf("CMP r%u, #%u\n", movCmpAddSub->Rd, movCmpAddSub->offset);
			#endif

			negative = true;
			Rd = r.find(movCmpAddSub->Rd);
			Rs = *Rd;
			val = Rs - movCmpAddSub->offset;
			break;

		//if COND {}
		case OpCode::B0:
		case OpCode::B1:

			#ifdef PRINT_INSTRUCTION
				printf("B%s #%i\n", Condition::names[condBranch->cond], condBranch->soffset * 2);
			#endif

  			if (doCondition((Condition::Value) condBranch->cond))
				r.pc += condBranch->soffset * 2;

			goto noConditionFlags;

		//{}
		case OpCode::B:
			
			#ifdef PRINT_INSTRUCTION
				printf("B #%i\n", branch->soffset * 2);
			#endif

			r.pc += branch->soffset * 2;
			goto noConditionFlags;

		default:

			EXCEPTION("OpCode was invalid or isn't implemented");
			return false;

	}

	//Set condition flags

	r.cpsr.zero = val == 0;
	r.cpsr.negative = (val & i32_MIN) != 0;

	//TODO: Overflow shouldn't happen when you change from -1 to 0, but it do

	r.cpsr.carry = (!negative) * (val < Rs);
	r.cpsr.overflow = (Rs & i32_MIN) != (val & i32_MIN);

	noConditionFlags:

	//Check if there's a next instruction (PC doesn't overflow and doesn't go out of bounds)

	r.pc += 2;

	noStep:

	return r.pc < buf.size && !(r.pc - 2 > r.pc);

}

inline bool Armulator::stepArm() {
	EXCEPTION("Armulator::stepArm not implemented yet");
	return false;
}