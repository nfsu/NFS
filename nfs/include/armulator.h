#pragma once
#include "generic.h"

namespace nfs {

	//Current Program Status Register
	struct CPSR {

		enum class Mode {

			USR,
			FIQ,
			IRQ,
			SVC,	//Monitor / supervisor

			ABT = 7,
			UND = 11,
			SYS = 15

		};

		union {

			u32 value = 0x10;

			struct {

				u32 mode : 4;
				u32 one : 1;				//If this is not one, it uses 26-bit mode
				u32 thumbMode : 1;
				u32 disableFIQ : 1;
				u32 disableIRQ : 1;
				u32 dataAbortDisable : 1;
				u32 isBigEndian : 1;
				u32 thumbIfThen0 : 6;
				u32 GE : 4;
				u32 nil : 4;
				u32 jazelle : 1;
				u32 overflow : 1;
				u32 carry : 1;
				u32 zero : 1;
				u32 negative : 1;

			};

		};

	};

	struct ArmRegisters {

		union {

			u32 d[16];

			struct {
				u32 r[15];
				u32 pc;
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

	};

	class Armulator {

	public:

		Armulator(Buffer buf, u32 entryPoint);

		CPSR &getCPSR();
		CPSR &getSPSR();
		ArmRegisters &getRegisters();

		void step();
		void exec();

		//u32 normally, u16 when thumbMode
		u8 *next();

		bool thumbMode();

	private:

		Buffer buf;

		ArmRegisters regs;
		CPSR cpsr, spsr;

	};

}