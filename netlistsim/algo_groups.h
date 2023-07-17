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
	pState->groupCount = 0;
	BitmapClear(pState->pBitmapGroup, pState->numNodes);
	TRACE_POP();
}

static inline void
GroupAdd(state_t *state, nodenum_t i)
{
	state->pGroupNodes[state->groupCount++] = i;
	BitmapSet(state->pBitmapGroup, i, 1);
}

static inline nodenum_t
GroupGet(state_t *state, count_t n)
{
	return state->pGroupNodes[n];
}

static inline bool
GroupContains(state_t *state, nodenum_t el)
{
	return BitmapGet(state->pBitmapGroup, el);
}

