/************************************************************
 *
 * Algorithms for Lists
 *
 ************************************************************/

static inline nodenum_t
listin_get(state_t *state, count_t i)
{
	return state->listIn.pNodes[i];
}

static inline count_t
ListInCount(state_t *state)
{
	return state->listIn.count;
}

static inline void
ListsSwitch(state_t *state)
{
	NodeList tmp = state->listIn;
	state->listIn = state->listOut;
	state->listOut = tmp;
}

static inline void
ListOutClear(state_t *state)
{

	state->listOut.count = 0;
	BitmapClear(state->listout_bitmap, state->numNodes);
}

static inline void
ListOutAdd(state_t *state, nodenum_t i)
{
	if (!BitmapGet(state->listout_bitmap, i)) 
	{
		state->listOut.pNodes[ state->listOut.count++ ] = i;
		
		BitmapSet(state->listout_bitmap, i, 1);
	}
}

