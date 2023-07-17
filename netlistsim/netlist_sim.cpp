/************************************************************
 *
 * Libc Functions and Basic Data Types
 *
 ************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "state.h"
#include "algo_bitmap.h"
#include "algo_lists.h"
#include "algo_groups.h"
#include "trace.h"

// creates a c1c2_t structure
static inline c1c2_t
c1c2(transnum_t tn, nodenum_t n)
{
	c1c2_t c = { tn, n };
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

static inline void
AddNodeToGroup(state_t *state, nodenum_t n)
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
	if (GroupContains(state, n))
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
	count_t end = state->pNodeC1C2Offset[n+1];

	for (count_t t = state->pNodeC1C2Offset[n]; t < end; t++) 
	{
		c1c2_t c = state->pNodeC1C2s[t];

		/* if the transistor connects c1 and c2... */
		if (state->onTransistors[c.transistor]) 
		{
			AddNodeToGroup(state, c.other_node);
		}
	}
	TRACE_POP();
}

static inline void
AddAllNodesToGroup(state_t *state, nodenum_t node)
{
	TRACE_PUSH("AddAllNodesToGroup");
	GroupClear(state);

	state->groupContainsValue = EGroupContainsValue::kNothing;

	AddNodeToGroup(state, node);
	TRACE_POP();
}

static inline bool
GetGroupValue(state_t *state)
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

static inline void
RecalcNode(state_t *state, nodenum_t node)
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
		nodenum_t nn = GroupGet(state, i);

		if (state->nodeState[nn] != newv) 
		{
			state->nodeState[nn] = newv;

			for (count_t t = 0; t < state->pNodeGateCount[nn]; t++) 
			{
				transnum_t tn = state->ppNodeGates[nn][t];
				state->onTransistors[tn] = newv;
			}

			if (newv) 
			{
				for (count_t g = 0; g < state->nodes_left_dependants[nn]; g++) 
				{
					ListOutAdd(state, state->nodes_left_dependant[nn][g]);
				}
			} 
			else 
			{
				for (count_t g = 0; g < state->nodes_dependants[nn]; g++) 
				{
					ListOutAdd(state, state->nodes_dependant[nn][g]);
				}
			}
		}
	}
	TRACE_POP();
}

void
RecalcNodeList(state_t *state)
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

		if (!ListInCount(state))
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
		for (count_t i = 0; i < ListInCount(state); i++) 
		{
			nodenum_t n = listin_get(state, i);
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

static inline void
add_nodes_dependant(state_t *state, nodenum_t a, nodenum_t b)
{
	for (count_t g = 0; g < state->nodes_dependants[a]; g++)

	if (state->nodes_dependant[a][g] == b)
	{
		return;
	}

	state->nodes_dependant[a][state->nodes_dependants[a]++] = b;
}

static inline void
add_nodes_left_dependant(state_t *state, nodenum_t a, nodenum_t b)
{
	for (count_t g = 0; g < state->nodes_left_dependants[a]; g++)
	if (state->nodes_left_dependant[a][g] == b)
	return;

	state->nodes_left_dependant[a][state->nodes_left_dependants[a]++] = b;
}

state_t *
SetupNodesAndTransistors(Transistor *pTransdefs, bool *node_is_pullup, nodenum_t numNodes, nodenum_t numTransistors, nodenum_t vss, nodenum_t vcc)
{
	/* allocate state */
	state_t *state = reinterpret_cast<state_t*>(malloc(sizeof(state_t)));

	state->numNodes = numNodes;
	state->numTransistors = numTransistors;
	state->vss = vss;
	state->vcc = vcc;

	// Bitmaps - large bit arrays
	state->pullupNodes.resize(state->numNodes);
	state->pulldownNodes.resize(state->numNodes);
	state->nodeState.resize(state->numNodes);

	// array of arrays - not sure what these do yet...
	state->ppNodeGates = reinterpret_cast<nodenum_t**>(malloc(state->numNodes * sizeof(*state->ppNodeGates)));
	for (count_t i = 0; i < state->numNodes; i++) 
	{
		state->ppNodeGates[i] = reinterpret_cast<nodenum_t*>(calloc(state->numNodes, sizeof(**state->ppNodeGates)));
	}

	state->pNodeGateCount = reinterpret_cast<count_t*>(calloc(state->numNodes, sizeof(*state->pNodeGateCount)));

	state->pNodeC1C2Offset = reinterpret_cast<count_t*>(calloc(state->numNodes + 1, sizeof(*state->pNodeC1C2Offset)));

	state->nodes_dependants = reinterpret_cast<nodenum_t*>(calloc(state->numNodes, sizeof(*state->nodes_dependants)));
	state->nodes_left_dependants = reinterpret_cast<nodenum_t*>(calloc(state->numNodes, sizeof(*state->nodes_left_dependants)));
	state->nodes_dependant = reinterpret_cast<nodenum_t**>(malloc(state->numNodes * sizeof(*state->nodes_dependant)));

	for (count_t i = 0; i < state->numNodes; i++) 
	{
		state->nodes_dependant[i] = reinterpret_cast<nodenum_t*>(calloc(state->numNodes, sizeof(**state->nodes_dependant)));
	}

	state->nodes_left_dependant = reinterpret_cast<nodenum_t**>(malloc(state->numNodes * sizeof(*state->nodes_left_dependant)));

	for (count_t i = 0; i < state->numNodes; i++) 
	{
		state->nodes_left_dependant[i] = reinterpret_cast<nodenum_t*>(calloc(state->numNodes, sizeof(**state->nodes_left_dependant)));
	}

	state->pTransistorsGate = reinterpret_cast<nodenum_t*>(calloc(state->numTransistors, sizeof(*state->pTransistorsGate)));
	state->pTransistorsC1 = reinterpret_cast<nodenum_t*>(calloc(state->numTransistors, sizeof(*state->pTransistorsC1)));
	state->pTransistorsC2 = reinterpret_cast<nodenum_t*>(calloc(state->numTransistors, sizeof(*state->pTransistorsC2)));
	state->onTransistors.resize(state->numTransistors);

	// list buffers
	state->pNodeList[0] = reinterpret_cast<nodenum_t*>(calloc(state->numNodes, sizeof(*state->pNodeList[0])));
	state->pNodeList[1] = reinterpret_cast<nodenum_t*>(calloc(state->numNodes, sizeof(*state->pNodeList[1])));

	state->listout_bitmap = reinterpret_cast<bitmap_t*>(calloc(BitmapGetRequiredSize(state->numNodes), sizeof(*state->listout_bitmap)));

	state->pGroupNodes = reinterpret_cast<nodenum_t*>(malloc(state->numNodes * sizeof(*state->pGroupNodes)));

	//state->groupBitmap.resize(state->numNodes);
	//state->groupBitmap.clear();
	state->pBitmapGroup = reinterpret_cast<bitmap_t*>(calloc(BitmapGetRequiredSize(state->numNodes), sizeof(*state->pBitmapGroup)));

	state->listIn.pNodes = state->pNodeList[0];
    state->listIn.count = 0;

	state->listOut.pNodes = state->pNodeList[1];
    state->listOut.count = 0;

	count_t i;
	/* copy nodes into r/w data structure */
	for (i = 0; i < state->numNodes; i++) 
	{
		state->pullupNodes[i] = node_is_pullup[i];
		state->pNodeGateCount[i] = 0;
	}

	/* copy transistors into r/w data structure */
	count_t j = 0;

	for (i = 0; i < state->numTransistors; i++) 
	{
		nodenum_t gate = pTransdefs[i].gate;
		nodenum_t c1 = pTransdefs[i].c1;
		nodenum_t c2 = pTransdefs[i].c2;
		/* skip duplicate transistors */
		bool found = false;

		for (count_t j2 = 0; j2 < j; j2++) 
		{
			if (state->pTransistorsGate[j2] == gate &&
				((state->pTransistorsC1[j2] == c1 &&
				  state->pTransistorsC2[j2] == c2) ||
				 (state->pTransistorsC1[j2] == c2 &&
				  state->pTransistorsC2[j2] == c1))) 
			{
				found = true;
			}
		}
		if (!found) 
		{
			state->pTransistorsGate[j] = gate;
			state->pTransistorsC1[j] = c1;
			state->pTransistorsC2[j] = c2;
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
		nodenum_t gate = state->pTransistorsGate[i];
		nodenum_t c1 = state->pTransistorsC1[i];
		nodenum_t c2 = state->pTransistorsC2[i];

		nodenum_t gateCount = state->pNodeGateCount[gate];
		state->ppNodeGates[gate][gateCount] = i;
		state->pNodeGateCount[gate]++;

		c1c2count[c1]++;
		c1c2count[c2]++;
		c1c2total += 2;
	}

	/* then sum the counts to find each node's offset into the c1c2 array */
	count_t c1c2offset = 0;

	for (i = 0; i < state->numNodes; i++) 
	{
		state->pNodeC1C2Offset[i] = c1c2offset;
		c1c2offset += c1c2count[i];
	}

	state->pNodeC1C2Offset[i] = c1c2offset;
	/* create and fill the nodes_c1c2s array according to these offsets */
	state->pNodeC1C2s = reinterpret_cast<c1c2_t*>(calloc(c1c2total, sizeof(*state->pNodeC1C2s)));
	memset(c1c2count, 0, state->numNodes * sizeof(*c1c2count));


	for (i = 0; i < state->numTransistors; i++) 
	{
		nodenum_t c1 = state->pTransistorsC1[i];
		nodenum_t c2 = state->pTransistorsC2[i];
		state->pNodeC1C2s[state->pNodeC1C2Offset[c1] + c1c2count[c1]++] = c1c2(i, c2);
		state->pNodeC1C2s[state->pNodeC1C2Offset[c2] + c1c2count[c2]++] = c1c2(i, c1);
	}

	free(c1c2count);

	for (i = 0; i < state->numNodes; i++) 
	{
		state->nodes_dependants[i] = 0;
		state->nodes_left_dependants[i] = 0;
		for (count_t g = 0; g < state->pNodeGateCount[i]; g++) 
		{
			transnum_t t = state->ppNodeGates[i][g];
			nodenum_t c1 = state->pTransistorsC1[t];
			if (c1 != vss && c1 != vcc) 
			{
				add_nodes_dependant(state, i, c1);
			}
			nodenum_t c2 = state->pTransistorsC2[t];
			if (c2 != vss && c2 != vcc) 
			{
				add_nodes_dependant(state, i, c2);
			}
			if (c1 != vss && c1 != vcc) 
			{
				add_nodes_left_dependant(state, i, c1);
			} 
			else 
			{
				add_nodes_left_dependant(state, i, c2);
			}
		}
	}

	return state;
}

void
DestroyNodesAndTransistors(state_t *state)
{
    for (count_t i = 0; i < state->numNodes; i++) 
	{
        free(state->ppNodeGates[i]);
    }

    free(state->ppNodeGates);
    free(state->pNodeC1C2s);
    free(state->pNodeGateCount);
    free(state->pNodeC1C2Offset);
    free(state->nodes_dependants);
    free(state->nodes_left_dependants);

    for (count_t i = 0; i < state->numNodes; i++) 
	{
        free(state->nodes_dependant[i]);
    }
    free(state->nodes_dependant);
    for (count_t i = 0; i < state->numNodes; i++)
	{
        free(state->nodes_left_dependant[i]);
    }
    free(state->nodes_left_dependant);
    free(state->pTransistorsGate);
    free(state->pTransistorsC1);
    free(state->pTransistorsC2);
    free(state->pNodeList[0]);
    free(state->pNodeList[1]);
    free(state->listout_bitmap);
    free(state->pGroupNodes);
    free(state->pBitmapGroup);
    free(state);
}

void
StabilizeChip(state_t *state)
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

void
SetNode(state_t *state, nodenum_t nodeNum, bool s)
{
	TRACE_PUSH("SetNode");
	state->pullupNodes[nodeNum] = s;
	state->pulldownNodes[nodeNum] = !s;

	ListOutAdd(state, nodeNum);

	RecalcNodeList(state);
	TRACE_POP();
}

bool
IsNodeHigh(state_t *state, nodenum_t nn)
{
	return state->nodeState[nn];
}

/************************************************************
 *
 * Interfacing and Extracting State
 *
 ************************************************************/

unsigned int
ReadNodes(state_t *state, int count, nodenum_t *nodelist)
{
	TRACE_PUSH("ReadNodes");
	int result = 0;
	for (int i = count - 1; i >= 0; i--) 
	{
		result <<=  1;
		result |= IsNodeHigh(state, nodelist[i]);
	}
	TRACE_POP();
	return result;
}

void
WriteNodes(state_t *state, int count, nodenum_t *nodelist, int v)
{
	TRACE_PUSH("WriteNodes");
	for (int i = 0; i < 8; i++, v >>= 1)
	{
		SetNode(state, nodelist[i], v & 1);
	}
	TRACE_POP();
}
