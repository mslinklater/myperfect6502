/************************************************************
 *
 * Algorithms for Lists
 *
 ************************************************************/

static inline nodenum_t
listin_get(state_t *state, count_t i)
{
	return state->listin.list[i];
}

static inline count_t
listin_count(state_t *state)
{
	return state->listin.count;
}

static inline void
lists_switch(state_t *state)
{
	list_t tmp = state->listin;
	state->listin = state->listout;
	state->listout = tmp;
}

static inline void
listout_clear(state_t *state)
{
	state->listout.count = 0;
	BitmapClear(state->listout_bitmap, state->numNodes);
}

static inline void
listout_add(state_t *state, nodenum_t i)
{
	if (!get_bitmap(state->listout_bitmap, i)) 
	{
		state->listout.list[state->listout.count++] = i;
		
		set_bitmap(state->listout_bitmap, i, 1);
	}
}

