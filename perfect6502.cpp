/*
 Copyright (c) 2010,2014 Michael Steil, Brian Silverman, Barry Silverman

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include <stdio.h>
#include <cassert>
#include "netlist_sim.h"
/* nodes & transistors */
#include "netlist_6502.h"
#include "netliststate.h"
#include "perfect6502.h"
#include "netlistsim.h"
#include "debug.h"

//------------------------------------------------------------------------------------------------------
/************************************************************
 *
 * 6502-specific Interfacing
 *
 ************************************************************/

NetListSim netListSim;

namespace Perfect6502
{

uint16_t
readAddressBus(state_t *state)
{
	nodenum_t nodes[] = { ab0, ab1, ab2, ab3, ab4, ab5, ab6, ab7, ab8, ab9, ab10, ab11, ab12, ab13, ab14, ab15 };
	return ReadNodes(state, 16, &nodes[0]);
}

uint8_t
readDataBus(state_t *state)
{
	nodenum_t nodes[] = { db0, db1, db2, db3, db4, db5, db6, db7 };
	return ReadNodes(state, 8, &nodes[0]);
}

void
writeDataBus(state_t *state, uint8_t d)
{
	nodenum_t nodes[] = { db0, db1, db2, db3, db4, db5, db6, db7 };
	WriteNodes(state, 8, &nodes[0], d);
}

bool
readRW(state_t *state)
{
	return IsNodeHigh(state, rw);
}

uint8_t
readA(state_t *state)
{
	nodenum_t nodes[] = { a0,a1,a2,a3,a4,a5,a6,a7 };
	return ReadNodes(state, 8, &nodes[0]);
}

uint8_t
readX(state_t *state)
{
	nodenum_t nodes[] = { x0,x1,x2,x3,x4,x5,x6,x7 };
	return ReadNodes(state, 8, &nodes[0]);
}

uint8_t
readY(state_t *state)
{
	nodenum_t nodes[] = { y0,y1,y2,y3,y4,y5,y6,y7 };
	return ReadNodes(state, 8, &nodes[0]);
}

uint8_t
readP(state_t *state)
{
	nodenum_t nodes[] = { p0,p1,p2,p3,p4,p5,p6,p7 };
	return ReadNodes(state, 8, &nodes[0]);
}

uint8_t
readIR(state_t *state)
{
	nodenum_t nodes[] = { notir0,notir1,notir2,notir3,notir4,notir5,notir6,notir7 };
	return ReadNodes(state, 8, &nodes[0]) ^ 0xFF;
}

uint8_t
readSP(state_t *state)
{
	nodenum_t nodes[] = { s0,s1,s2,s3,s4,s5,s6,s7 };
	return ReadNodes(state, 8, &nodes[0]);
}

uint8_t
readPCL(state_t *state)
{
	nodenum_t nodes[] = { pcl0,pcl1,pcl2,pcl3,pcl4,pcl5,pcl6,pcl7 };
	return ReadNodes(state, 8, &nodes[0]);
}

uint8_t
readPCH(state_t *state)
{
	nodenum_t nodes[] = { pch0,pch1,pch2,pch3,pch4,pch5,pch6,pch7 };
	return ReadNodes(state, 8, &nodes[0]);
}

uint16_t
readPC(state_t *state)
{
	return (readPCH(state) << 8) | readPCL(state);
}

/************************************************************
 *
 * Address Bus and Data Bus Interface
 *
 ************************************************************/

uint8_t memory[65536];

static uint8_t
mRead(uint16_t a)
{
	return memory[a];
}

static void
mWrite(uint16_t a, uint8_t d)
{
	memory[a] = d;
}

static inline void HandleMemory(state_t *state)
{
	int bp=0;

	// OLD
	if (IsNodeHigh(state, rw))
		writeDataBus(state, mRead(readAddressBus(state)));
	else
		mWrite(readAddressBus(state), readDataBus(state));
	
	// NEW
	if(netListSim.IsNodeHigh(rw))
	{
		bp++;
	}
	else
	{
		bp++;
	}
}

/************************************************************
 *
 * Main Clock Loop
 *
 ************************************************************/

unsigned int cycle;

void
step(state_t *state)
{
	bool clk = IsNodeHigh(state, clk0);

	/* invert clock */
	SetNode(state, clk0, !clk);
	RecalcNodeList(state);

	{
		netListSim.SetNode(clk0, !clk);
		netListSim.RecalcNodeList();
	}

	/* handle memory reads and writes */
	if (!clk)
		HandleMemory(state);

	cycle++;
}

state_t *
InitAndResetChip()	// copied
{
	state_t *state = SetupNodesAndTransistors(netlist_6502_transdefs,
										   netlist_6502_node_is_pullup,
										   vss,
										   vcc);
	netListSim.SetupNodesAndTransistors(netlist_6502_transdefs, netlist_6502_node_is_pullup, vss, vcc);

	SetNode(state, res, 0);
	netListSim.SetNode(res, false);

	SetNode(state, clk0, 1);
	netListSim.SetNode(clk0, true);

	assert(Debug::CheckCores(state, netListSim));

	SetNode(state, rdy, 1);
	netListSim.SetNode(rdy, true);

	SetNode(state, so, 0);
	netListSim.SetNode(so, false);

	SetNode(state, irq, 1);
	netListSim.SetNode(irq, true);

	SetNode(state, nmi, 1);
	netListSim.SetNode(nmi, true);


	StabilizeChip(state);
	netListSim.StabilizeChip();


//	{	// Init and reset the new chip



//	}

	// save both strings out so we can do some investigation


	// check chips are same state

	/* hold RESET for 8 cycles */
	for (int i = 0; i < 16; i++)
	{
		step(state);
	}

	/* release RESET */
	SetNode(state, res, 1);
	RecalcNodeList(state);

	{
		netListSim.SetNode(res, true);
		netListSim.RecalcNodeList();
	}

	cycle = 0;

	return state;
}

void
destroyChip(state_t *state)
{
    DestroyNodesAndTransistors(state);
}

/************************************************************
 *
 * Tracing/Debugging
 *
 ************************************************************/

void
chipStatus(state_t *state)
{
	bool clk = IsNodeHigh(state, clk0);
	uint16_t a = readAddressBus(state);
	uint8_t d = readDataBus(state);
	bool r_w = IsNodeHigh(state, rw);

	if (clk) {
		printf("%d AB:%04X D:%02X RnW:%d PC:%04X A:%02X X:%02X Y:%02X SP:%02X P:%02X IR:%02X",
		   cycle/2,
		  // clk,
		   a,
		   d,
		   r_w,
		   readPC(state),
		   readA(state),
		   readX(state),
		   readY(state),
		   readSP(state),
		   readP(state),
		   readIR(state));

		if (r_w)
			printf(" R$%04X=$%02X", a, memory[a]);
		else
			printf(" W$%04X=$%02X", a, d);
		printf("\n");
	}
}

}
