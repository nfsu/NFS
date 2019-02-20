#pragma once
#include "generic.h"

namespace nfs {

	namespace arm {

		//Current Program Status Register
		struct CPSR {

			enum class Mode {

				USR26 = 0x00,
				FIQ26,
				IRQ26,
				SVC26,

				USR = 0x10,
				FIQ,
				IRQ,
				SVC,	//Monitor / supervisor

				ABT = 0x17,
				UND = 0x1B,
				SYS = 0x1F

			};

			union {

				u32 value = 0x10;

				struct {

					u32 mode : 5;
					u32 thumbMode : 1;
					u32 disableFIQ : 1;
					u32 disableIRQ : 1;
					u32 dataAbortDisable : 1;
					u32 isBigEndian : 1;
					u32 thumbIfThen0 : 6;
					u32 GE : 4;
					u32 nil : 4;
					u32 thumbIfThen1 : 2;
					u32 jazelle : 1;
					u32 overflow : 1;
					u32 carry : 1;
					u32 zero : 1;
					u32 negative : 1;

				};

			};

			void printState();

		};

		//TODO: https://stackoverflow.com/questions/19724406/concept-of-bank-registers-in-arm
		//Some registers are different in other modes
		struct Registers {

			union {

				u32 r[16]{};

				struct {
					u32 reg[13];
					u32 sp;		//Stack pointer
					u32 lr;		//Link register
					u32 pc;		//Program counter
				};

			};

			void printState();

			u32 &operator[](size_t i);

		};

		struct Condition {

			enum Value {
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

			static constexpr char names[][3] = {
				"EQ", "NE", "CS", "CC",
				"MI", "PL", "VS", "VC",
				"HI", "LS", "GE", "LT",
				"GT", "LE", "AL", "NV"
			};

		};

		namespace thumb {

			enum class OpCode : u8 {

				//Format 1: move shifted register
				LSL,					//Logical Shift Left
				LSR,					//Logical Shift Right
				ASR,					//Arithmetic Shift Right

				//Format 2: add/subtract
				ADD_SUB,				//Add or subtract (5-bit register or constant)

				//Format 3: move/compare/add/subtract

				MOV,					//Move from memory
				CMP,					//Compare
				ADD,					//Add const (8-bit)
				SUB,					//Sub const (8-bit)

				//Format 5.16: Conditional branch
				B0 = 0x1A,				//BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC
				B1 = 0x1B				//BHI, BLS, BGE, BLT, BGT, BLE, BAL (always), BNE = software interrupt

			};

			union Op {
				u16 value;

				struct {
					u16 contents : 11;
					u16 code : 5;
				};
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
			};

			union MovCmpAddSub {
				u16 value;

				struct {

					u16 offset : 8;
					u16 Rd : 3;
					u16 opCode : 5;

				};
			};

			union CondBranch {
				u16 value;

				struct {
					i16 soffset : 8;
					u16 cond : 4;
					u16 opCode : 4;
				};
			};

		}


		//For more documentation, check out https://ece.uwaterloo.ca/~ece222/ARM/ARM7-TDMI-manual-pt3.pdf
		class Armulator {

		public:

			Armulator(Buffer buf, u32 entryPoint);

			CPSR &getCPSR();
			CPSR &getSPSR();
			Registers &getRegisters();
			u8 *next();
			bool thumbMode();

			bool step();
			void exec();

			inline bool condition(Condition::Value condition);

			void printState();

		private:

			inline bool stepThumb();
			inline bool stepArm();

		private:

			Buffer buf;

			Registers r;
			CPSR cpsr, spsr;

		};

	}

}