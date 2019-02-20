#pragma once
#include "generic.h"

namespace nfs {

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

	struct ArmRegisters {

		union {

			u32 d[16]{};

			struct {
				u32 r[15];
				u32 pc;		//Program counter
			};

			struct FIQ {
				u32 r[8];
				u32 rfiq[7];
				u32 pc;
			} fiq;

			struct IRQ_SVC_ABT_UND {
				u32 r[13];
				u32 rReserved[2];
				u32 pc;
			} irq, svc, abt, und;

		};

		void printState();

		u32 &operator[](size_t i);

	};

	enum class ArmThumbOpCodes : u8 {

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


	};

	struct ArmThumbOp {

		union {

			u16 value;

			struct {
				u16 contents : 11;
				u16 opCode : 5;
			};

		};

	};

	struct ArmThumbRegOp {
		union {

			u16 value;

			struct {
				u16 Rd : 3;			//Destination
				u16 Rs : 3;			//Source
				u16 pad : 5;
				u16 opCode : 5;
			};

		};

	};

	struct ArmThumbShift {
		
		union {

			u16 value;

			struct {

				u16 Rd : 3;			//Destination
				u16 Rs : 3;			//Source
				u16 offset : 5;		//Shift count
				u16 opCode : 5;

			};

		};

	};

	struct ArmThumbAddSub {
		
		union {

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

	};

	struct ArmThumbMovCmpAddSub {
		
		union {

			u16 value;

			struct {

				u16 offset : 8;
				u16 Rd : 3;
				u16 opCode : 5;

			};

		};

	};

	class Armulator {

	public:

		Armulator(Buffer buf, u32 entryPoint);

		CPSR &getCPSR();
		CPSR &getSPSR();
		ArmRegisters &getRegisters();
		u8 *next();
		bool thumbMode();

		bool step();
		void exec();

		void printState();

	private:

		inline bool stepThumb();
		inline bool stepArm();

	private:

		Buffer buf;

		ArmRegisters r;
		CPSR cpsr, spsr;

	};

}