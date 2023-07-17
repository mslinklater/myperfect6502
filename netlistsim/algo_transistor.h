/************************************************************
 *
 * Algorithms for Transistors
 *
 ************************************************************/

static inline void
SetTransistorOn(state_t *state, transnum_t t, bool s)
{
	BitmapSet(state->pBitmapOnTransistors, t, s);
}

static inline bool
GetTransistorOn(state_t *state, transnum_t t)
{
	return BitmapGet(state->pBitmapOnTransistors, t);
}

