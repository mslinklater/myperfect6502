#include <stdio.h>

#include "../perfect6502.h"
/* XXX hook up memory[] with RAM[] in runtime.c */
 
/************************************************************
 *
 * Interface to OS Library Code / Monitor
 *
 ************************************************************/

extern int kernal_dispatch();

/* imported by runtime.c */
unsigned char A, X, Y, S, P;
unsigned short PC;
int N, Z, C;

void
InitMonitor(char* filename, int address, int isBasic)
{
	// Load the ROM

	FILE *f;
	f = fopen(filename, "r");
	fseek(f, 0L, SEEK_END);
	int fileSize = ftell(f);
	fseek(f, 0L, SEEK_SET);
	fread(memory + address, 1, fileSize, f);
	fclose(f);

	if(isBasic)
	{
		// CBM BASIC ROM is loaded from 0xa000-0xe4b7 (17591 bytes)
		// 0xa000-0xbfff 8K BASIC	(8192)
		// 0xc000-0xcfff FREE RAM	(4096)
		// 0xd000-0xdfff stuff
		// 0xe000-0xe4b7 - partial Operating System KERNAL ROM - needed for the startup message,
		// which is at 0xe460

		/*
		* fill the KERNAL jumptable with JMP $F800;
		* we will put code there later that loads
		* the CPU state and returns
		*/
		for (unsigned short addr = 0xFF90; addr < 0xFFF3; addr += 3) {
			memory[addr+0] = 0x4C;
			memory[addr+1] = 0x00;
			memory[addr+2] = 0xF8;
		}

		/*
		* cbmbasic scribbles over 0x01FE/0x1FF, so we can't start
		* with a stackpointer of 0 (which seems to be the state
		* after a RESET), so RESET jumps to 0xF000, which contains
		* a JSR to the actual start of cbmbasic (0xe394 ?)
		*/
		memory[0xf000] = 0x20;
		memory[0xf001] = 0x94;
		memory[0xf002] = 0xE3;
		
		// Set the reset vector to 0xf000
		memory[0xfffc] = 0x00;
		memory[0xfffd] = 0xF0;
	}
}

void
handle_monitor(state_t *state)
{
	PC = readPC(state);

	if (PC >= 0xFF90 && ((PC - 0xFF90) % 3 == 0)) {
		/* get register status out of 6502 */
		A = readA(state);
		X = readX(state);
		Y = readY(state);
		S = readSP(state);
		P = readP(state);
		N = P >> 7;
		Z = (P >> 1) & 1;
		C = P & 1;

		kernal_dispatch();

		/* encode processor status */
		P &= 0x7C; /* clear N, Z, C */
		P |= (N << 7) | (Z << 1) | C;

		/*
		 * all KERNAL calls make the 6502 jump to $F800, so we
		 * put code there that loads the return state of the
		 * KERNAL function and returns to the caller
		 */
		memory[0xf800] = 0xA9; /* LDA #P */
		memory[0xf801] = P;
		memory[0xf802] = 0x48; /* PHA    */
		memory[0xf803] = 0xA9; /* LHA #A */
		memory[0xf804] = A;
		memory[0xf805] = 0xA2; /* LDX #X */
		memory[0xf806] = X;
		memory[0xf807] = 0xA0; /* LDY #Y */
		memory[0xf808] = Y;
		memory[0xf809] = 0x28; /* PLP    */
		memory[0xf80a] = 0x60; /* RTS    */
		/*
		 * XXX we could do RTI instead of PLP/RTS, but RTI seems to be
		 * XXX broken in the chip dump - after the KERNAL call at 0xFF90,
		 * XXX the 6502 gets heavily confused about its program counter
		 * XXX and executes garbage instructions
		 */
	}
}

