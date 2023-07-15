/************************************************************
 *
 * Algorithms for Nodes
 *
 ************************************************************/

/*
 * The "value" propertiy of VCC and GND is never evaluated in the code,
 * so we don't bother initializing it properly or special-casing writes.
 */

static inline void
set_nodes_pullup(state_t *state, transnum_t t, BOOL s)
{
	BitmapSet(state->pPullupNodesBitmap, t, s);
}

static inline BOOL
get_nodes_pullup(state_t *state, transnum_t t)
{
	return BitmapGet(state->pPullupNodesBitmap, t);
}

static inline void
set_nodes_pulldown(state_t *state, transnum_t t, BOOL s)
{
	BitmapSet(state->pPulldownNodesBitmap, t, s);
}

static inline BOOL
get_nodes_pulldown(state_t *state, transnum_t t)
{
	return BitmapGet(state->pPulldownNodesBitmap, t);
}

static inline void
set_nodes_value(state_t *state, transnum_t t, BOOL s)
{
	BitmapSet(state->pNodesStateBitmap, t, s);
}

static inline BOOL
get_nodes_value(state_t *state, transnum_t t)
{
	return BitmapGet(state->pNodesStateBitmap, t);
}

