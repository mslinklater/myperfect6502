#include "../perfect6502.h"
#include "runtime.h"
#include "runtime_init.h"
#include "stdio.h"
#include "stdlib.h"

int
main( int argv, char** argc)
{
	int clk = 0;
	int numClocksToRun = atoi(argc[3]);

	void* pState = InitAndResetChip();

	/* set up memory for user program */
	init_monitor(argc[1], atoi(argc[2]));

	/* emulate the 6502! */
	for (int i=0 ; i < numClocksToRun ; ++i) 
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
}
