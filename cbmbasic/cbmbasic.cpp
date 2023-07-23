#include "../perfect6502.h"
//#include "runtime.h"
#include "runtime_init.h"
#include "stdio.h"
#include "stdlib.h"
#include "netlistsim.h"

#include "netlist_6502.h"

int
main( int argv, char** argc)
{
	int clk = 0;
	char* strRom = argc[1];
	int address = atoi(argc[2]);
	int numClocksToRun = atoi(argc[3]);
	int isBasic = atoi(argc[4]);

	NetListSim netListSim;

	state_t* pState = InitAndResetChip();	// perfect6502.cpp

	{	// Init and reset the new chip
		netListSim.SetupNodesAndTransistors(netlist_6502_transdefs, netlist_6502_node_is_pullup, vss, vcc);
		netListSim.SetNode(res, false);
		netListSim.SetNode(clk0, true);
		netListSim.SetNode(rdy, true);
		netListSim.SetNode(so, false);
		netListSim.SetNode(irq, true);
		netListSim.SetNode(nmi, true);

		netListSim.StabilizeChip();

		for (int i = 0; i < 16; i++)
		{
			bool clk = netListSim.IsNodeHigh(clk0);

			/* invert clock */
			netListSim.SetNode(clk0, !clk);
			netListSim.RecalcNodeList();
		}
		netListSim.SetNode(res, true);
		netListSim.RecalcNodeList();
	}

	/* set up memory for user program */
	Runtime::InitMonitor(strRom, address, isBasic);

	static int i;

	/* emulate the 6502! */
	for (i=0 ; i < numClocksToRun ; ++i) 
	{
		step(pState);
		clk = !clk;
		if (clk)
		{
			Runtime::HandleMonitor(pState);
		}
//		chipStatus(pState);

//		if (!(i % 1000)) printf("%d\n", i);
	};
	destroyChip(pState);
}
