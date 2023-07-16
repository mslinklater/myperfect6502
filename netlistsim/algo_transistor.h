/************************************************************
 *
 * Algorithms for Transistors
 *
 ************************************************************/

static inline void
SetTransistorOn(state_t *state, transnum_t t, BOOL s)
{
	BitmapSet(state->pBitmapOnTransistors, t, s);
}

static inline BOOL
GetTransistorOn(state_t *state, transnum_t t)
{
	return BitmapGet(state->pBitmapOnTransistors, t);
}

