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
	for(int i=0 ; i<state->listIn.count ; i++)
		state->listoutBitmap[state->listIn.pNodes[i]] = false;
	state->listOut.count = 0;
}

static inline void
ListOutAdd(state_t *state, nodenum_t i)
{
	if (!state->listoutBitmap[i]) 
	{
		state->listOut.pNodes[ state->listOut.count++ ] = i;
		
		state->listoutBitmap[i] = true;
	}
}

