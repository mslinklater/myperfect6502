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

static inline void
GroupClear(state_t *pState)
{
	pState->groupcount = 0;
	BitmapClear(pState->groupbitmap, pState->numNodes);
}

static inline void
group_add(state_t *state, nodenum_t i)
{
	state->group[state->groupcount++] = i;
	BitmapSet(state->groupbitmap, i, 1);
}

static inline nodenum_t
group_get(state_t *state, count_t n)
{
	return state->group[n];
}

static inline BOOL
group_contains(state_t *state, nodenum_t el)
{
	return BitmapGet(state->groupbitmap, el);
}

static inline count_t
group_count(state_t *state)
{
	return state->groupcount;
}

