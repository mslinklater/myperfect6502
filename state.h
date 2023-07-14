#pragma once

/************************************************************
 *
 * Main State Data Structure
 *
 ************************************************************/

typedef struct {
	nodenum_t numNodes;
	nodenum_t numTransistors;
	nodenum_t vss;
	nodenum_t vcc;

	/* everything that describes a node */
	bitmap_t *pPullupNodesBitmap;		// which ndoes are pullup
	bitmap_t *pPulldownNodesBitmap;		// which nodes are pulldown
	bitmap_t *pNodesStateBitmap;		// Node state held in a bit array

	nodenum_t **nodes_gates;	// num-nodes size array of pointers... each one points to num-nodes array of shorts

	c1c2_t *nodes_c1c2s;

	count_t *nodes_gatecount;
	count_t *nodes_c1c2offset;

	nodenum_t *nodes_dependants;
	nodenum_t *nodes_left_dependants;
	nodenum_t **nodes_dependant;
	nodenum_t **nodes_left_dependant;

	/* everything that describes a transistor */
	nodenum_t *transistors_gate;
	nodenum_t *transistors_c1;
	nodenum_t *transistors_c2;
	bitmap_t *transistors_on;

	/* the nodes we are working with */
	nodenum_t *list1;
	list_t listin;

	/* the indirect nodes we are collecting for the next run */
	nodenum_t *list2;
	list_t listout;

	bitmap_t *listout_bitmap;

	nodenum_t *group;
	count_t groupcount;
	bitmap_t *groupbitmap;

	enum {
		contains_nothing,
		contains_hi,
		contains_pullup,
		contains_pulldown,
		contains_vcc,
		contains_vss
	} group_contains_value;
} state_t;
