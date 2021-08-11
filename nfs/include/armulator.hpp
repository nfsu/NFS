#pragma once
#include "generic.hpp"

namespace nfs {

	namespace arm {

		//Current Program Status Register
		struct CPSR {

			enum class Mode {

				USR = 0x10,
				FIQ,
				IRQ,
				SVC,	//Monitor / supervisor

				ABT = 0x17,
				UND = 0x1B,
				SYS = 0x1F
			};

			//Gets the type for the id (used for SPSR and registers)
			static constexpr u8 modeId[] = {
				0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,
				0,1,2,3,0,0,0,4,
				0,0,0,5,0,0,0,0
			};

			union {

				u32 value = 0x10;

				//TODO: Avoid bitflags

				struct {
					u32 mode : 5;
					u32 thumbMode : 1;
					u32 disableFIQ : 1;
					u32 disableIRQ : 1;
					u32 padding : 20;
					u32 overflow : 1;
					u32 carry : 1;
					u32 zero : 1;
					u32 negative : 1;
				};
			};

			void printState();
		};

		struct RegisterBank {

			union {

				u32 dat[36];

				struct {
					u32 sysUsr[15];
					u32 pc;				//Program counter
					u32 fiq[7];
					u32 irq[2];
					u32 svc[2];
					u32 abt[2];
					u32 und[2];
					CPSR cpsr;
					CPSR spsr[6];
				};
			};

			//Lookup table for the registers using their mode id
			static constexpr u8 registerMapping[][16] = {
				{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },	//SYS and USR
				{ 0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 15 },	//FIQ
				{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 23, 24, 15 },	//IRQ
				{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 25, 26, 15 },	//SVC
				{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 27, 28, 15 },	//ABT
				{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 29, 30, 15 },	//UND
			};

			void printState();

			u32 &operator[](usz i);
			u32 *find(usz i);
			u32 &get(usz i);

		};

		enum ConditionValue {
			EQ,				//Zero is set (==)
			NE,				//!EQ (!=)
			CS,				//Carry is set (unsigned >=)
			CC,				//!CS (unsigned <)
			MI,				//Negative is set (< 0)
			PL,				//!MI (>= 0)
			VS,				//oVerflow (set)
			VC,				//!VS (clear)
			HI,				//Carry & !Zero (unsigned >)
			LS,				//!HI (unsigned <=)
			GE,				//Negative == Overflow (>=)
			LT,				//!GE (<)
			GT,				//!Zero && Negative == Overflow  (>)
			LE,				//!GT (<=)
			AL,				//Always
			NV				//Software interrupt (never)
		};

		struct Condition {

			static constexpr char names[][3] = {
				"EQ", "NE", "CS", "CC",
				"MI", "PL", "VS", "VC",
				"HI", "LS", "GE", "LT",
				"GT", "LE", "AL", "NV"
			};

		};

		namespace thumb {

			enum ThumbRegister : u8 {
				r0, r1, r2, r3, r4, r5, r6, r7
			};

			enum class OpCode : u8 {

				//Format 1: Move shifted register
				//LSL rx, ry, #offset5
				//LSR rx, ry, #offset5
				//ASR rx, ry, #offset5
				LSL,					//Logical Shift Left
				LSR,					//Logical Shift Right
				ASR,					//Arithmetic Shift Right

				//Format 2: Add/subtract
				//ADD rx, ry, rz
				//ADD rx, ry, #offset3
				//SUB rx, ry, rz
				//SUB rx, ry, #offset3
				ADD_SUB,				//Add or subtract (5-bit register or constant)

				//Format 3: Move/compare/add/subtract
				//MOV rx, #offset8
				//CMP rx, #offset8
				//ADD rx, #offset8
				//SUB rx, #offset8
				MOV,					//Move from memory
				CMP,					//Compare
				ADD,					//Add const (8-bit)
				SUB,					//Sub const (8-bit)

				//Format 16: Conditional branch
				//B{cond} #soffset8
				B0 = 0x1A,				//BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC
				B1 = 0x1B,				//BHI, BLS, BGE, BLT, BGT, BLE, BAL (always), BNV = software interrupt

				//Format 18: Unconditional branch
				//B #soffset11
				B = 0x1C,

				//Format 19: Branch with link
				//BL #soffset23
				BLH = 0x1E,				//First 11 bits
				BLL = 0x1F				//Second 11 bits
			};

			union Op {

				u16 value;

				struct {
					u16 contents : 11;
					u16 code : 5;
				};

				static constexpr u16 mov(ThumbRegister dst, u8 value);
				static constexpr u16 cmp(ThumbRegister dst, u8 value);

				static constexpr u16 add(ThumbRegister dst, u8 value);
				static constexpr u16 add(ThumbRegister dst, ThumbRegister value);
				static constexpr u16 add(ThumbRegister dst, ThumbRegister src, ThumbRegister value);
				static constexpr u16 add(ThumbRegister dst, ThumbRegister src, u8 value);

				static constexpr u16 sub(ThumbRegister dst, u8 value);
				static constexpr u16 sub(ThumbRegister dst, ThumbRegister value);
				static constexpr u16 sub(ThumbRegister dst, ThumbRegister src, ThumbRegister value);
				static constexpr u16 sub(ThumbRegister dst, ThumbRegister src, u8 value);

				static constexpr u16 lsl(ThumbRegister dst, ThumbRegister src, u8 value);
				static constexpr u16 lsl(ThumbRegister dst, u8 value);

				static constexpr u16 lsr(ThumbRegister dst, ThumbRegister src, u8 value);
				static constexpr u16 lsr(ThumbRegister dst, u8 value);

				static constexpr u16 asr(ThumbRegister dst, ThumbRegister src, u8 value);
				static constexpr u16 asr(ThumbRegister dst, u8 value);

				//TODO: Test negative BL and B

				static constexpr u16 b(ConditionValue cond, i8 value);
				static constexpr u16 b(i16 value);
				static constexpr u16 bl(u16 value, bool highBits);

			};

			union RegOp {

				u16 value;

				struct {
					u16 Rd : 3;			//Destination
					u16 Rs : 3;			//Source
					u16 pad : 5;
					u16 code : 5;
				};
			};

			union Shift {

				u16 value;

				struct {
					u16 Rd : 3;			//Destination
					u16 Rs : 3;			//Source
					u16 offset : 5;		//Shift count
					u16 code : 5;
				};

				static constexpr u16 op(ThumbRegister dst, ThumbRegister src, u8 value, OpCode code);
			};

			union AddSub {

				u16 value;

				struct {
					u16 Rd : 3;			//Destination
					u16 Rs : 3;			//Source
					u16 Rn : 3;			//Second register or value
					u16 sub : 1;
					u16 intermediate : 1;	//0 = register, 1 = value
					u16 opCode : 5;
				};

				static constexpr u16 op(ThumbRegister dst, ThumbRegister src, u8 value, bool isNegative);
				static constexpr u16 op(ThumbRegister dst, ThumbRegister src, ThumbRegister value, bool isNegative);
			};

			union MovCmpAddSub {

				u16 value;

				struct {
					u16 offset : 8;
					u16 Rd : 3;
					u16 opCode : 5;
				};

				static constexpr u16 op(ThumbRegister dst, u8 value, OpCode code);
			};

			union CondBranch {

				u16 value;

				struct {
					i16 soffset : 8;
					u16 cond : 4;
					u16 opCode : 4;
				};
			};

			union Branch {

				u16 value;

				struct {
					i16 soffset : 11;
					u16 opCode : 5;
				};
			};

			union LongBranch {

				u16 value;

				struct {
					u16 offset : 11;
					u16 opCode : 5;
				};
			};

			//All operations

			constexpr u16 MovCmpAddSub::op(ThumbRegister dst, u8 value, OpCode code) {
				MovCmpAddSub op{};
				op.opCode = u16(code);
				op.Rd = u16(dst);
				op.offset = value;
				return op.value;
			}

			constexpr u16 Shift::op(ThumbRegister dst, ThumbRegister src, u8 value, OpCode code) {
				Shift op{};
				op.code = u16(code);
				op.Rd = u16(dst);
				op.Rs = u16(src);
				op.offset = value;
				return op.value;
			}

			//Relative branch (-128 to 127 instructions)
			constexpr u16 Op::b(ConditionValue cond, i8 value) {
				CondBranch op{};
				op.opCode = u16(OpCode::B0) >> 1;
				op.cond = u16(cond);
				op.soffset = value;
				return op.value;
			}

			//Unconditional branch (jump; -1024 to 1023 instructions)
			constexpr u16 Op::b(i16 value) {
				Branch op{};
				op.opCode = u16(OpCode::B);
				op.soffset = value;
				return op.value;
			}

			constexpr u16 Op::bl(u16 value, bool highBits) {
				LongBranch op{};
				op.opCode = (u16)(highBits ? OpCode::BLH : OpCode::BLL);
				op.offset = highBits ? (value >> 11) : value;
				return op.value;
			}

			constexpr u16 AddSub::op(ThumbRegister dst, ThumbRegister src, u8 value, bool isNegative) {
				AddSub op{};
				op.opCode = u16(OpCode::ADD_SUB);
				op.sub = u16(isNegative);
				op.intermediate = u16(true);
				op.Rd = u16(dst);
				op.Rs = u16(src);
				op.Rn = u16(value);
				return op.value;
			}

			constexpr u16 AddSub::op(ThumbRegister dst, ThumbRegister src, ThumbRegister value, bool isNegative) {
				AddSub op{};
				op.opCode = u16(OpCode::ADD_SUB);
				op.sub = u16(isNegative);
				op.intermediate = u16(false);
				op.Rd = u16(dst);
				op.Rs = u16(src);
				op.Rn = u16(value);
				return op.value;
			}

			constexpr u16 Op::mov(ThumbRegister dst, u8 value) {
				return MovCmpAddSub::op(dst, value, OpCode::MOV);
			}

			constexpr u16 Op::cmp(ThumbRegister dst, u8 value) {
				return MovCmpAddSub::op(dst, value, OpCode::CMP);
			}

			constexpr u16 Op::add(ThumbRegister dst, u8 value) {
				return MovCmpAddSub::op(dst, value, OpCode::ADD);
			}

			constexpr u16 Op::sub(ThumbRegister dst, u8 value) {
				return MovCmpAddSub::op(dst, value, OpCode::SUB);
			}

			constexpr u16 Op::lsl(ThumbRegister dst, ThumbRegister src, u8 value) {
				return Shift::op(dst, src, value, OpCode::LSL);
			}

			constexpr u16 Op::lsr(ThumbRegister dst, ThumbRegister src, u8 value) {
				return Shift::op(dst, src, value, OpCode::LSR);
			}

			constexpr u16 Op::asr(ThumbRegister dst, ThumbRegister src, u8 value) {
				return Shift::op(dst, src, value, OpCode::ASR);
			}

			constexpr u16 Op::lsl(ThumbRegister dst, u8 value) {
				return Shift::op(dst, dst, value, OpCode::LSL);
			}

			constexpr u16 Op::lsr(ThumbRegister dst, u8 value) {
				return Shift::op(dst, dst, value, OpCode::LSR);
			}

			constexpr u16 Op::asr(ThumbRegister dst, u8 value) {
				return Shift::op(dst, dst, value, OpCode::ASR);
			}

			constexpr u16 Op::add(ThumbRegister dst, ThumbRegister src, ThumbRegister value) {
				return AddSub::op(dst, src, value, false);
			}

			constexpr u16 Op::add(ThumbRegister dst, ThumbRegister src, u8 value) {
				return AddSub::op(dst, src, value, false);
			}

			constexpr u16 Op::add(ThumbRegister dst, ThumbRegister value) {
				return add(dst, dst, value);
			}

			constexpr u16 Op::sub(ThumbRegister dst, ThumbRegister src, ThumbRegister value) {
				return AddSub::op(dst, src, value, true);
			}

			constexpr u16 Op::sub(ThumbRegister dst, ThumbRegister src, u8 value) {
				return AddSub::op(dst, src, value, true);
			}

			constexpr u16 Op::sub(ThumbRegister dst, ThumbRegister value) {
				return sub(dst, dst, value);
			}
		}

		//For more documentation, check out https://ece.uwaterloo.ca/~ece222/ARM/ARM7-TDMI-manual-pt3.pdf
		class Armulator {

		public:

			Armulator(Buffer buf, u32 entryPoint);

			inline CPSR &getCPSR() { return r.cpsr; }
			inline RegisterBank &getRegisterBank() { return r; }
			inline u8 *next() { return buf.add(r.pc); }
			inline bool thumbMode() { return r.cpsr.thumbMode; }

			bool step();
			void exec();

			void printState();
			bool condition(ConditionValue condition);

		private:

			inline bool doCondition(ConditionValue condition);
			inline bool doStep();
			inline bool stepThumb();
			inline bool stepArm();

			inline void IRQ();
			inline void FIQ();
			inline void restore();
			inline void switchMode(CPSR::Mode mode);

		private:

			Buffer buf;

			RegisterBank r{
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,
				0x10,0x10,0x10,0x10 
			};
		};
	}
}