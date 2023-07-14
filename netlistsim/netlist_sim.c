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
#include "algo_node.h"
#include "algo_transistor.h"
#include "algo_lists.h"
#include "algo_groups.h"

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

#define INCLUDED_FROM_NETLIST_SIM_C
#include "netlist_sim.h"
#undef INCLUDED_FROM_NETLIST_SIM_C

/************************************************************
 *
 * Node and Transistor Emulation
 *
 ************************************************************/

static inline void
AddNodeToGroup(state_t *state, nodenum_t n)
{
	/*
	 * We need to stop at vss and vcc, otherwise we'll revisit other groups
	 * with the same value - just because they all derive their value from
	 * the fact that they are connected to vcc or vss.
	 */
	if (n == state->vss) 
	{
		state->group_contains_value = contains_vss;
		return;
	}
	
	if (n == state->vcc) 
	{
		if (state->group_contains_value != contains_vss)
		{
			state->group_contains_value = contains_vcc;
		}
		return;
	}

	if (group_contains(state, n))
	{
		return;
	}

	group_add(state, n);

	if (state->group_contains_value < contains_pulldown && get_nodes_pulldown(state, n)) 
	{
		state->group_contains_value = contains_pulldown;
	}
	if (state->group_contains_value < contains_pullup && get_nodes_pullup(state, n)) 
	{
		state->group_contains_value = contains_pullup;
	}
	if (state->group_contains_value < contains_hi && get_nodes_value(state, n)) 
	{
		state->group_contains_value = contains_hi;
	}

	/* revisit all transistors that control this node */
	count_t end = state->nodes_c1c2offset[n+1];

	for (count_t t = state->nodes_c1c2offset[n]; t < end; t++) 
	{
		c1c2_t c = state->nodes_c1c2s[t];

		/* if the transistor connects c1 and c2... */
		if (get_transistors_on(state, c.transistor)) 
		{
			AddNodeToGroup(state, c.other_node);
		}
	}
}

static inline void
AddAllNodesToGroup(state_t *state, nodenum_t node)
{
	group_clear(state);

	state->group_contains_value = contains_nothing;

	AddNodeToGroup(state, node);
}

static inline BOOL
getGroupValue(state_t *state)
{
	switch (state->group_contains_value) 
	{
		case contains_vcc:
		case contains_pullup:
		case contains_hi:
			return YES;
		case contains_vss:
		case contains_pulldown:
		case contains_nothing:
			return NO;
	}
	return NO;
}

static inline void
recalcNode(state_t *state, nodenum_t node)
{
	/*
	 * get all nodes that are connected through
	 * transistors, starting with this one
	 */
	AddAllNodesToGroup(state, node);

	/* get the state of the group */
	BOOL newv = getGroupValue(state);

	/*
	 * - set all nodes to the group state
	 * - check all transistors switched by nodes of the group
	 * - collect all nodes behind toggled transistors
	 *   for the next run
	 */
	for (count_t i = 0; i < group_count(state); i++) {
		nodenum_t nn = group_get(state, i);
		if (get_nodes_value(state, nn) != newv) {
			set_nodes_value(state, nn, newv);
			for (count_t t = 0; t < state->nodes_gatecount[nn]; t++) {
				transnum_t tn = state->nodes_gates[nn][t];
				set_transistors_on(state, tn, newv);
			}

			if (newv) {
				for (count_t g = 0; g < state->nodes_left_dependants[nn]; g++) {
					listout_add(state, state->nodes_left_dependant[nn][g]);
				}
			} else {
				for (count_t g = 0; g < state->nodes_dependants[nn]; g++) {
					listout_add(state, state->nodes_dependant[nn][g]);
				}
			}
		}
	}
}

void
recalcNodeList(state_t *state)
{
	for (int j = 0; j < 100; j++) {	/* loop limiter */
		/*
		 * make the secondary list our primary list, use
		 * the data storage of the primary list as the
		 * secondary list
		 */
		lists_switch(state);

		if (!listin_count(state))
			break;

		listout_clear(state);

		/*
		 * for all nodes, follow their paths through
		 * turned-on transistors, find the state of the
		 * path and assign it to all nodes, and re-evaluate
		 * all transistors controlled by this path, collecting
		 * all nodes that changed because of it for the next run
		 */
		for (count_t i = 0; i < listin_count(state); i++) {
			nodenum_t n = listin_get(state, i);
			recalcNode(state, n);
		}
	}
	listout_clear(state);
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
	return;

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
setupNodesAndTransistors(Transistor *transdefs, BOOL *node_is_pullup, nodenum_t numNodes, nodenum_t numTransistors, nodenum_t vss, nodenum_t vcc)
{
	/* allocate state */
	state_t *state = malloc(sizeof(state_t));
	state->numNodes = numNodes;
	state->numTransistors = numTransistors;
	state->vss = vss;
	state->vcc = vcc;
	state->pPullupNodesBitmap = calloc(BitmapGetRequiredSize(state->numNodes), sizeof(*state->pPullupNodesBitmap));
	state->pPulldownNodesBitmap = calloc(BitmapGetRequiredSize(state->numNodes), sizeof(*state->pPulldownNodesBitmap));
	state->nodes_value = calloc(BitmapGetRequiredSize(state->numNodes), sizeof(*state->nodes_value));
	state->nodes_gates = malloc(state->numNodes * sizeof(*state->nodes_gates));

	for (count_t i = 0; i < state->numNodes; i++) {
		state->nodes_gates[i] = calloc(state->numNodes, sizeof(**state->nodes_gates));
	}

	state->nodes_gatecount = calloc(state->numNodes, sizeof(*state->nodes_gatecount));
	state->nodes_c1c2offset = calloc(state->numNodes + 1, sizeof(*state->nodes_c1c2offset));
	state->nodes_dependants = calloc(state->numNodes, sizeof(*state->nodes_dependants));
	state->nodes_left_dependants = calloc(state->numNodes, sizeof(*state->nodes_left_dependants));
	state->nodes_dependant = malloc(state->numNodes * sizeof(*state->nodes_dependant));
	for (count_t i = 0; i < state->numNodes; i++) {
		state->nodes_dependant[i] = calloc(state->numNodes, sizeof(**state->nodes_dependant));
	}
	state->nodes_left_dependant = malloc(state->numNodes * sizeof(*state->nodes_left_dependant));
	for (count_t i = 0; i < state->numNodes; i++) {
		state->nodes_left_dependant[i] = calloc(state->numNodes, sizeof(**state->nodes_left_dependant));
	}
	state->transistors_gate = calloc(state->numTransistors, sizeof(*state->transistors_gate));
	state->transistors_c1 = calloc(state->numTransistors, sizeof(*state->transistors_c1));
	state->transistors_c2 = calloc(state->numTransistors, sizeof(*state->transistors_c2));
	state->transistors_on = calloc(BitmapGetRequiredSize(state->numTransistors), sizeof(*state->transistors_on));
	state->list1 = calloc(state->numNodes, sizeof(*state->list1));
	state->list2 = calloc(state->numNodes, sizeof(*state->list2));
	state->listout_bitmap = calloc(BitmapGetRequiredSize(state->numNodes), sizeof(*state->listout_bitmap));
	state->group = malloc(state->numNodes * sizeof(*state->group));
	state->groupbitmap = calloc(BitmapGetRequiredSize(state->numNodes), sizeof(*state->groupbitmap));
	state->listin.list = state->list1;
        state->listin.count = 0;
	state->listout.list = state->list2;
        state->listout.count = 0;

	count_t i;
	/* copy nodes into r/w data structure */
	for (i = 0; i < state->numNodes; i++) {
		set_nodes_pullup(state, i, node_is_pullup[i]);
		state->nodes_gatecount[i] = 0;
	}
	/* copy transistors into r/w data structure */
	count_t j = 0;
	for (i = 0; i < state->numTransistors; i++) {
		nodenum_t gate = transdefs[i].gate;
		nodenum_t c1 = transdefs[i].c1;
		nodenum_t c2 = transdefs[i].c2;
		/* skip duplicate transistors */
		BOOL found = NO;
		for (count_t j2 = 0; j2 < j; j2++) {
			if (state->transistors_gate[j2] == gate &&
				((state->transistors_c1[j2] == c1 &&
				  state->transistors_c2[j2] == c2) ||
				 (state->transistors_c1[j2] == c2 &&
				  state->transistors_c2[j2] == c1))) {
					 found = YES;
				 }
		}
		if (!found) {
			state->transistors_gate[j] = gate;
			state->transistors_c1[j] = c1;
			state->transistors_c2[j] = c2;
			j++;
		}
	}
	state->numTransistors = j;

	/* cross reference transistors in nodes data structures */
	/* start by computing how many c1c2 entries should be created for each node */
	count_t *c1c2count = calloc(state->numNodes, sizeof(*c1c2count));
	count_t c1c2total = 0;
	for (i = 0; i < state->numTransistors; i++) {
		nodenum_t gate = state->transistors_gate[i];
		state->nodes_gates[gate][state->nodes_gatecount[gate]++] = i;
		c1c2count[state->transistors_c1[i]]++;
		c1c2count[state->transistors_c2[i]]++;
		c1c2total += 2;
	}
	/* then sum the counts to find each node's offset into the c1c2 array */
	count_t c1c2offset = 0;
	for (i = 0; i < state->numNodes; i++) {
		state->nodes_c1c2offset[i] = c1c2offset;
		c1c2offset += c1c2count[i];
	}
	state->nodes_c1c2offset[i] = c1c2offset;
	/* create and fill the nodes_c1c2s array according to these offsets */
	state->nodes_c1c2s = calloc(c1c2total, sizeof(*state->nodes_c1c2s));
	memset(c1c2count, 0, state->numNodes * sizeof(*c1c2count));
	for (i = 0; i < state->numTransistors; i++) {
		nodenum_t c1 = state->transistors_c1[i];
		nodenum_t c2 = state->transistors_c2[i];
		state->nodes_c1c2s[state->nodes_c1c2offset[c1] + c1c2count[c1]++] = c1c2(i, c2);
		state->nodes_c1c2s[state->nodes_c1c2offset[c2] + c1c2count[c2]++] = c1c2(i, c1);
	}
	free(c1c2count);

	for (i = 0; i < state->numNodes; i++) {
		state->nodes_dependants[i] = 0;
		state->nodes_left_dependants[i] = 0;
		for (count_t g = 0; g < state->nodes_gatecount[i]; g++) {
			transnum_t t = state->nodes_gates[i][g];
			nodenum_t c1 = state->transistors_c1[t];
			if (c1 != vss && c1 != vcc) {
				add_nodes_dependant(state, i, c1);
			}
			nodenum_t c2 = state->transistors_c2[t];
			if (c2 != vss && c2 != vcc) {
				add_nodes_dependant(state, i, c2);
			}
			if (c1 != vss && c1 != vcc) {
				add_nodes_left_dependant(state, i, c1);
			} else {
				add_nodes_left_dependant(state, i, c2);
			}
		}
	}

#if 0 /* unnecessary - RESET will stabilize the network anyway */
	/* all nodes are down */
	for (nodenum_t nn = 0; nn < state->nodes; nn++) {
		set_nodes_value(state, nn, 0);
	}
	/* all transistors are off */
	for (transnum_t tn = 0; tn < state->transistors; tn++)
	set_transistors_on(state, tn, NO);
#endif

	return state;
}

void
DestroyNodesAndTransistors(state_t *state)
{
    free(state->pPullupNodesBitmap);
    free(state->pPulldownNodesBitmap);
    free(state->nodes_value);

    for (count_t i = 0; i < state->numNodes; i++) 
	{
        free(state->nodes_gates[i]);
    }

    free(state->nodes_gates);
    free(state->nodes_c1c2s);
    free(state->nodes_gatecount);
    free(state->nodes_c1c2offset);
    free(state->nodes_dependants);
    free(state->nodes_left_dependants);

    for (count_t i = 0; i < state->numNodes; i++) 
	{
        free(state->nodes_dependant[i]);
    }
    free(state->nodes_dependant);
    for (count_t i = 0; i < state->numNodes; i++) {
        free(state->nodes_left_dependant[i]);
    }
    free(state->nodes_left_dependant);
    free(state->transistors_gate);
    free(state->transistors_c1);
    free(state->transistors_c2);
    free(state->transistors_on);
    free(state->list1);
    free(state->list2);
    free(state->listout_bitmap);
    free(state->group);
    free(state->groupbitmap);
    free(state);
}

void
stabilizeChip(state_t *state)
{
	for (count_t i = 0; i < state->numNodes; i++)
	{
		listout_add(state, i);
	}

	recalcNodeList(state);
}

/************************************************************
 *
 * Node State
 *
 ************************************************************/

void
setNode(state_t *state, nodenum_t nn, BOOL s)
{
	set_nodes_pullup(state, nn, s);
	set_nodes_pulldown(state, nn, !s);

	listout_add(state, nn);

	recalcNodeList(state);
}

BOOL
isNodeHigh(state_t *state, nodenum_t nn)
{
	return get_nodes_value(state, nn);
}

/************************************************************
 *
 * Interfacing and Extracting State
 *
 ************************************************************/

unsigned int
readNodes(state_t *state, int count, nodenum_t *nodelist)
{
	int result = 0;
	for (int i = count - 1; i >= 0; i--) 
	{
		result <<=  1;
		result |= isNodeHigh(state, nodelist[i]);
	}
	return result;
}

void
writeNodes(state_t *state, int count, nodenum_t *nodelist, int v)
{
	for (int i = 0; i < 8; i++, v >>= 1)
	setNode(state, nodelist[i], v & 1);
}
