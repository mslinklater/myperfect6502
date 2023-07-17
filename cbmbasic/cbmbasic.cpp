#include "../perfect6502.h"
#include "runtime.h"
#include "runtime_init.h"
#include "stdio.h"
#include "stdlib.h"

int
main( int argv, char** argc)
{
	int clk = 0;
	int address = atoi(argc[2]);
	int numClocksToRun = atoi(argc[3]);
	int isBasic = atoi(argc[4]);

	state_t* pState = InitAndResetChip();

	/* set up memory for user program */
	init_monitor(argc[1], address, isBasic);

	static int i;

	/* emulate the 6502! */
	for (i=0 ; i < numClocksToRun ; ++i) 
	{
		step(pState);
		clk = !clk;
		if (clk)
		{
			handle_monitor(pState);
		}
//		chipStatus(pState);

		//if (!(i % 1000)) printf("%d\n", i);
	};
	int bp=0;
	bp++;
}
