/************************************************************
 *
 * Algorithms for Transistors
 *
 ************************************************************/

static inline void
SetTransistorsOn(state_t *state, transnum_t t, BOOL s)
{
	BitmapSet(state->pBitmapOnTransistors, t, s);
}

static inline BOOL
GetTransistorsOn(state_t *state, transnum_t t)
{
	return BitmapGet(state->pBitmapOnTransistors, t);
}

