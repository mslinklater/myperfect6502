/************************************************************
 *
 * Algorithms for Bitmaps
 *
 ************************************************************/

// How many 64-bit values are required to build a bit-mask for this many things...
static inline uint32_t BitmapGetRequiredSize(uint32_t in)
{
	uint32_t out = in / (sizeof(bitmap_t) * 8) + 1;
	return out;
}

static inline void
BitmapClear(bitmap_t *bitmap, count_t count)
{
	memset(bitmap, 0, BitmapGetRequiredSize(count)*sizeof(bitmap_t));
}

static inline void
BitmapSet(bitmap_t *bitmap, int index, BOOL state)
{
	if (state)
		bitmap[index>>BITMAP_SHIFT] |= ONE << (index & BITMAP_MASK);
	else
		bitmap[index>>BITMAP_SHIFT] &= ~(ONE << (index & BITMAP_MASK));
}

static inline BOOL
BitmapGet(bitmap_t *bitmap, int index)
{
	return (bitmap[index>>BITMAP_SHIFT] >> (index & BITMAP_MASK)) & 1;
}

