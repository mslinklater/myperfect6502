#include "../perfect6502.h"
//#include "runtime.h"
#include "runtime_init.h"
#include "stdio.h"
#include "stdlib.h"
//#include "netlistsim.h"

#include "netlist_6502.h"

int
main( int argv, char** argc)
{
	int clk = 0;
	char* strRom = argc[1];
	int address = atoi(argc[2]);
	int numClocksToRun = atoi(argc[3]);
	int isBasic = atoi(argc[4]);


	state_t* pState = Perfect6502::InitAndResetChip();	// perfect6502.cpp

	/* set up memory for user program */
	Runtime::InitMonitor(strRom, address, isBasic);

	static int i;

	/* emulate the 6502! */
	for (i=0 ; i < numClocksToRun ; ++i) 
	{
		Perfect6502::step(pState);
		clk = !clk;
		if (clk)
		{
			Runtime::HandleMonitor(pState);
		}
//		chipStatus(pState);

//		if (!(i % 1000)) printf("%d\n", i);
	};
	Perfect6502::destroyChip(pState);
}
