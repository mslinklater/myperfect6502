/************************************************************
 *
 * Libc Functions and Basic Data Types
 *
 ************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include "netliststate.h"
#include "algo_lists.h"
#include "algo_groups.h"
#include "trace.h"

// creates a C1C2 structure - measure this...
static inline C1C2
NewC1C2(transnum_t tn, nodenum_t n)
{
	C1C2 c = { tn, n };
	return c;
}

/************************************************************
 *
 * Main Header Include
 *
 ************************************************************/

#include "netlist_sim.h"

/************************************************************
 *
 * Node and Transistor Emulation
 *
 ************************************************************/

static inline void AddNodeToGroup(state_t *state, nodenum_t n)
{
	TRACE_PUSH("AddNodeToGroup");
	/*
	 * We need to stop at vss and vcc, otherwise we'll revisit other groups
	 * with the same value - just because they all derive their value from
	 * the fact that they are connected to vcc or vss.
	 */
	if (n == state->vss) 
	{
		state->groupContainsValue = EGroupContainsValue::kVss;
		
		TRACE_POP();
		return;
	}
	
	if (n == state->vcc) 
	{
		if (state->groupContainsValue != EGroupContainsValue::kVss)
		{
			state->groupContainsValue = EGroupContainsValue::kVcc;
		}
		TRACE_POP();
		return;
	}

	// If group already contains this node, return
	if (state->groupBitmap[n])	//GroupContains(state, n))
	{
		TRACE_POP();
		return;
	}

	GroupAdd(state, n);

	if (state->groupContainsValue < EGroupContainsValue::kPulldown && state->pulldownNodes[n]) 
	{
		state->groupContainsValue = EGroupContainsValue::kPulldown;
	}
	if (state->groupContainsValue < EGroupContainsValue::kPullup && state->pullupNodes[n]) 
	{
		state->groupContainsValue = EGroupContainsValue::kPullup;
	}
	if (state->groupContainsValue < EGroupContainsValue::kHigh && state->nodeState[n]) 
	{
		state->groupContainsValue = EGroupContainsValue::kHigh;
	}

	/* revisit all transistors that control this node */
	count_t end = state->nodeC1C2Offset[n+1];

	for (count_t t = state->nodeC1C2Offset[n]; t < end; t++) 
	{
		C1C2 c = state->nodeC1C2s[t];

		/* if the transistor connects c1 and c2... */
		if (state->onTransistors[c.transistor]) 
		{
			AddNodeToGroup(state, c.other_node);
		}
	}
	TRACE_POP();
}

static inline void AddAllNodesToGroup(state_t *state, nodenum_t node)
{
	TRACE_PUSH("AddAllNodesToGroup");
	GroupClear(state);

	state->groupContainsValue = EGroupContainsValue::kNothing;

	AddNodeToGroup(state, node);
	TRACE_POP();
}

static inline bool GetGroupValue(state_t *state)
{
	switch (state->groupContainsValue) 
	{
		case EGroupContainsValue::kVcc:
		case EGroupContainsValue::kPullup:
		case EGroupContainsValue::kHigh:
			return true;
		case EGroupContainsValue::kVss:
		case EGroupContainsValue::kPulldown:
		case EGroupContainsValue::kNothing:
			return false;
	}
	return false;
}

static inline void RecalcNode(state_t *state, nodenum_t node)
{
	TRACE_PUSH("RecalcNode");
	/*
	 * get all nodes that are connected through
	 * transistors, starting with this one
	 */
	AddAllNodesToGroup(state, node);

	/* get the state of the group */
	bool newv = GetGroupValue(state);

	/*
	 * - set all nodes to the group state
	 * - check all transistors switched by nodes of the group
	 * - collect all nodes behind toggled transistors
	 *   for the next run
	 */
	for (count_t i = 0; i < state->groupCount; i++) 
	{
		nodenum_t nn = state->groupNodes[i];	//GroupGet(state, i);

		if (state->nodeState[nn] != newv) 
		{
			state->nodeState[nn] = newv;

			for (count_t t = 0; t < state->nodeGateCount[nn]; t++) 
			{
				transnum_t tn = state->nodeGates[nn][t];
				state->onTransistorsCount[tn]++;
				state->onTransistors[tn] = newv;
			}

			if (newv) 
			{
				for (count_t g = 0; g < state->nodesLeftDeps[nn]; g++) 
				{
					ListOutAdd(state, state->nodesLeftDependant[nn][g]);
				}
			} 
			else 
			{
				for (count_t g = 0; g < state->nodesDeps[nn]; g++) 
				{
					ListOutAdd(state, state->nodesDependant[nn][g]);
				}
			}
		}
	}
	TRACE_POP();
}

void RecalcNodeList(state_t *state)
{
	TRACE_PUSH("RecalcNodeList");

	for (int j = 0; j < 100; j++) 
	{	/* loop limiter */
		TRACE_PUSH("RecalcNodeList - Inner");
		/*
		 * make the secondary list our primary list, use
		 * the data storage of the primary list as the
		 * secondary list
		 */
		ListsSwitch(state);

		if (!state->listIn.count)	//ListInCount(state))
		{
			TRACE_POP();
			break;
		}

		ListOutClear(state);

		/*
		 * for all nodes, follow their paths through
		 * turned-on transistors, find the state of the
		 * path and assign it to all nodes, and re-evaluate
		 * all transistors controlled by this path, collecting
		 * all nodes that changed because of it for the next run
		 */
		for (count_t i = 0; i < state->listIn.count; i++) 
		{
			nodenum_t n = state->listIn.pNodes[i];	//listin_get(state, i);
			RecalcNode(state, n);
		}
		TRACE_POP();
	}
	ListOutClear(state);
	TRACE_POP();
}

/************************************************************
 *
 * Initialization
 *
 ************************************************************/

static inline void AddNodesDependant(state_t *state, nodenum_t a, nodenum_t b)	// copied
{
	for (count_t g = 0; g < state->nodesDeps[a]; g++)
	{
		if (state->nodesDependant[a][g] == b)
		{
			return;
		}
	}

	state->nodesDependant[a][state->nodesDeps[a]++] = b;
}

static inline void AddNodesLeftDependant(state_t *state, nodenum_t a, nodenum_t b) // copied
{
	for (count_t g = 0; g < state->nodesLeftDeps[a]; g++)
	{
		if (state->nodesLeftDependant[a][g] == b)
		{
			return;
		}
	}

	state->nodesLeftDependant[a][state->nodesLeftDeps[a]++] = b;
}

state_t* SetupNodesAndTransistors(std::vector<Transistor>& transdefs, std::vector<bool>& node_is_pullup, nodenum_t vss, nodenum_t vcc) // copied
{
	/* allocate state */
//	state_t *state = reinterpret_cast<state_t*>(malloc(sizeof(state_t)));
	state_t* state = new state_t;

	state->maxGroupCount = 0;

	state->numNodes = node_is_pullup.size();
	state->numTransistors = transdefs.size();
	state->vss = vss;
	state->vcc = vcc;

	// Bitmaps - large bit arrays
	state->pullupNodes.resize(state->numNodes);
	state->pulldownNodes.resize(state->numNodes);

	// array of arrays - not sure what these do yet...
	state->nodeGates.resize(state->numNodes);
	for (count_t i = 0; i < state->numNodes; i++) 
	{
		state->nodeGates[i].resize(state->numNodes);
	}

	state->nodeState.resize(state->numNodes);

	state->nodeGateCount.resize(state->numNodes);

	state->nodeC1C2Offset.resize(state->numNodes + 1);

	state->nodesDeps.resize(state->numNodes);
	state->nodesLeftDeps.resize(state->numNodes);
	state->nodesDependant.resize(state->numNodes);

	for (count_t i = 0; i < state->numNodes; i++) 
	{
		state->nodesDependant[i].resize(state->numNodes);
	}

	state->nodesLeftDependant.resize(state->numNodes);

	for (count_t i = 0; i < state->numNodes; i++) 
	{
		state->nodesLeftDependant[i].resize(state->numNodes);
	}

	state->transistorsGate.resize(state->numTransistors);
	state->transistorsC1.resize(state->numTransistors);
	state->transistorsC2.resize(state->numTransistors);
	state->onTransistors.resize(state->numTransistors);
	state->onTransistorsCount.resize(state->numTransistors);

	// list buffers
	state->nodeList[0].resize(state->numNodes);
	state->nodeList[1].resize(state->numNodes);

	state->listoutBitmap.resize(state->numNodes);

	state->groupNodes.resize(state->numNodes);

	state->groupBitmap.resize(state->numNodes);

	state->listIn.pNodes = state->nodeList[0].data();
    state->listIn.count = 0;

	state->listOut.pNodes = state->nodeList[1].data();
    state->listOut.count = 0;

	count_t i;
	/* copy nodes into r/w data structure */
	for (i = 0; i < state->numNodes; i++) 
	{
		state->pullupNodes[i] = node_is_pullup[i];
		state->nodeGateCount[i] = 0;
	}

	/* copy transistors into r/w data structure, skipping any duplicates found */
	count_t j = 0;

	for (i = 0; i < state->numTransistors; i++) 
	{
		nodenum_t gate = transdefs[i].gate;
		nodenum_t c1 = transdefs[i].c1;
		nodenum_t c2 = transdefs[i].c2;
		/* skip duplicate transistors */
		bool found = false;

		for (count_t j2 = 0; j2 < j; j2++) 
		{
			if (state->transistorsGate[j2] == gate &&
				((state->transistorsC1[j2] == c1 &&
				  state->transistorsC2[j2] == c2) ||
				 (state->transistorsC1[j2] == c2 &&
				  state->transistorsC2[j2] == c1))) 
			{
				found = true;
			}
		}
		if (!found) 
		{
			state->transistorsGate[j] = gate;
			state->transistorsC1[j] = c1;
			state->transistorsC2[j] = c2;
			j++;
		}
	}
	state->numTransistors = j;

	/* cross reference transistors in nodes data structures */
	/* start by computing how many c1c2 entries should be created for each node */
	count_t *c1c2count = reinterpret_cast<count_t*>(calloc(state->numNodes, sizeof(*c1c2count)));
	count_t c1c2total = 0;

	for (i = 0; i < state->numTransistors; i++) 
	{
		nodenum_t gate = state->transistorsGate[i];
		nodenum_t c1 = state->transistorsC1[i];
		nodenum_t c2 = state->transistorsC2[i];

		nodenum_t gateCount = state->nodeGateCount[gate];
		state->nodeGates[gate][gateCount] = i;
		state->nodeGateCount[gate]++;

		c1c2count[c1]++;
		c1c2count[c2]++;
		c1c2total += 2;
	}

	/* then sum the counts to find each node's offset into the c1c2 array */
	count_t c1c2offset = 0;

	for (i = 0; i < state->numNodes; i++) 
	{
		state->nodeC1C2Offset[i] = c1c2offset;
		c1c2offset += c1c2count[i];
	}

	state->nodeC1C2Offset[i] = c1c2offset;
	/* create and fill the nodes_c1c2s array according to these offsets */
	state->nodeC1C2s.resize(c1c2total);
	memset(c1c2count, 0, state->numNodes * sizeof(*c1c2count));

	for (i = 0; i < state->numTransistors; i++) 
	{
		nodenum_t c1 = state->transistorsC1[i];
		nodenum_t c2 = state->transistorsC2[i];
		state->nodeC1C2s[state->nodeC1C2Offset[c1] + c1c2count[c1]++] = NewC1C2(i, c2);
		state->nodeC1C2s[state->nodeC1C2Offset[c2] + c1c2count[c2]++] = NewC1C2(i, c1);
	}

	free(c1c2count);

	for (i = 0; i < state->numNodes; i++) 
	{
		state->nodesDeps[i] = 0;
		state->nodesLeftDeps[i] = 0;
		for (count_t g = 0; g < state->nodeGateCount[i]; g++) 
		{
			transnum_t t = state->nodeGates[i][g];
			nodenum_t c1 = state->transistorsC1[t];
			if (c1 != vss && c1 != vcc) 
			{
				AddNodesDependant(state, i, c1);
			}
			nodenum_t c2 = state->transistorsC2[t];
			if (c2 != vss && c2 != vcc) 
			{
				AddNodesDependant(state, i, c2);
			}
			if (c1 != vss && c1 != vcc) 
			{
				AddNodesLeftDependant(state, i, c1);
			} 
			else 
			{
				AddNodesLeftDependant(state, i, c2);
			}
		}
	}

	return state;
}

void DestroyNodesAndTransistors(state_t *state)
{
    delete state;
}

void StabilizeChip(state_t *state)
{
	TRACE_PUSH("StabilizeChip");
	// Add every node to the out list... stabilize *everything*
	for (count_t i = 0; i < state->numNodes; i++)
	{
		ListOutAdd(state, i);
	}

	RecalcNodeList(state);
	TRACE_POP();
}

/************************************************************
 *
 * Node State
 *
 ************************************************************/


std::string GetStateString(state_t *state)
{
	std::string ret;

	AddBitArrayToStateString(ret, state->nodeState);
	AddBitArrayToStateString(ret, state->pullupNodes);
	AddBitArrayToStateString(ret, state->pulldownNodes);
	AddBitArrayToStateString(ret, state->onTransistors);

	for(auto outer : state->nodeGates)
		for(count_t c : outer)
			ret.append(std::to_string(c));

	AddNodeNumArrayToStateString(ret, state->nodesDeps);
	AddNodeNumArrayToStateString(ret, state->nodesLeftDeps);

	for(auto outer : state->nodesDependant)
		for(count_t c : outer)
			ret.append(std::to_string(c));
	for(auto outer : state->nodesLeftDependant)
		for(count_t c : outer)
			ret.append(std::to_string(c));

	AddNodeNumArrayToStateString(ret, state->transistorsGate);
	AddNodeNumArrayToStateString(ret, state->transistorsC1);
	AddNodeNumArrayToStateString(ret, state->transistorsC2);
	AddNodeNumArrayToStateString(ret, state->groupNodes);

	return ret;
}

void SetNode(state_t *state, nodenum_t nodeNum, bool s)
{
	TRACE_PUSH("SetNode");
//	if(s != state->pullupNodes[nodeNum])	// possible optimisation ? Or a fix... taken from https://github.com/hoglet67/perfect6502/commit/aed0d9a3c37cebb48956c7ab9a3dc4ec11e8d862
	{
		state->pullupNodes[nodeNum] = s;
		state->pulldownNodes[nodeNum] = !s;

		ListOutAdd(state, nodeNum);

		RecalcNodeList(state);
	}
	TRACE_POP();
}

bool IsNodeHigh(state_t *state, nodenum_t nn)
{
	return state->nodeState[nn];
}

/************************************************************
 *
 * Interfacing and Extracting State
 *
 ************************************************************/

unsigned int ReadNodes(state_t *state, int count, nodenum_t *nodelist)
{
	TRACE_PUSH("ReadNodes");
	assert(count <= 32);	// for the int

	int result = 0;
	for (int i = count - 1; i >= 0; i--) 
	{
		result <<=  1;
		result |= IsNodeHigh(state, nodelist[i]);
	}
	TRACE_POP();
	return result;
}

void WriteNodes(state_t *state, int count, nodenum_t *nodelist, int v)
{
	TRACE_PUSH("WriteNodes");
	assert(count <= 32);	// for the int

	for (int i = 0; i < 8; i++, v >>= 1)
	{
		SetNode(state, nodelist[i], v & 1);
	}
	TRACE_POP();
}
