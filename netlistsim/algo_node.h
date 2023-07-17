/************************************************************
 *
 * Algorithms for Nodes
 *
 ************************************************************/

/*
 * The "value" propertiy of VCC and GND is never evaluated in the code,
 * so we don't bother initializing it properly or special-casing writes.
 */
#if 0
static inline void
SetNodePullup(state_t *state, transnum_t t, bool s)
{
	state->pullupNodesBitmap[t] = s;
//	BitmapSet(state->pPullupNodesBitmap, t, s);
}

static inline bool
GetNodePullup(state_t *state, transnum_t t)
{
	return state->pullupNodesBitmap[t];
//	return BitmapGet(state->pPullupNodesBitmap, t);
}
#endif

static inline void
SetNodePulldown(state_t *state, transnum_t t, bool s)
{
	BitmapSet(state->pPulldownNodesBitmap, t, s);
}

static inline bool
GetNodePulldown(state_t *state, transnum_t t)
{
	return BitmapGet(state->pPulldownNodesBitmap, t);
}

static inline void
SetNodeState(state_t *state, transnum_t t, bool s)
{
	BitmapSet(state->pNodesStateBitmap, t, s);
}

static inline bool
GetNodeState(state_t *state, transnum_t t)
{
	return BitmapGet(state->pNodesStateBitmap, t);
}

