/************************************************************
 *
 * Algorithms for Groups of Nodes
 *
 ************************************************************/

/*
 * a group is a set of connected nodes, which consequently
 * share the same value
 *
 * we use an array and a count for O(1) insert and
 * iteration, and a redundant bitmap for O(1) lookup
 */

#include "trace.h"

static inline void
GroupClear(state_t *pState)
{
	TRACE_PUSH("GroupClear");
	for(int i=0 ; i<pState->groupCount ; i++)
		pState->groupBitmap[pState->groupNodes[i]] = false;
	pState->groupCount = 0;
	TRACE_POP();
}

static inline void
GroupAdd(state_t *state, nodenum_t i)
{
	state->groupNodes[state->groupCount++] = i;
	state->groupBitmap[i] = true;
}

static inline nodenum_t
GroupGet(state_t *state, count_t n)
{
	return state->groupNodes[n];
}

static inline bool
GroupContains(state_t *state, nodenum_t el)
{
	return state->groupBitmap[el];
}

