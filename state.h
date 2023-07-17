#pragma once

/************************************************************
 *
 * Main State Data Structure
 *
 ************************************************************/

enum EGroupContainsValue {
	kNothing,
	kHigh,
	kPullup,
	kPulldown,
	kVcc,
	kVss
};

struct  state_t {
	nodenum_t numNodes;
	nodenum_t numTransistors;
	nodenum_t vss;
	nodenum_t vcc;

	/* everything that describes a node */
	bitmap_t *pPullupNodesBitmap;		// which ndoes are pullup
	bitmap_t *pPulldownNodesBitmap;		// which nodes are pulldown
	bitmap_t *pNodesStateBitmap;		// Node state held in a bit array

	nodenum_t **ppNodeGates;	// num-nodes size array of pointers... each one points to num-nodes array of shorts

	c1c2_t *pNodeC1C2s;

	count_t *pNodeGateCount;

	// Offsets in to the C1C2s array
	count_t *pNodeC1C2Offset;

	nodenum_t *nodes_dependants;
	nodenum_t *nodes_left_dependants;

	nodenum_t **nodes_dependant;
	nodenum_t **nodes_left_dependant;

	/* everything that describes a transistor */
	nodenum_t *pTransistorsGate;
	nodenum_t *pTransistorsC1;
	nodenum_t *pTransistorsC2;
	bitmap_t *pBitmapOnTransistors;

	// QUESTION - Can we have more list to multi-thread ?

	/* the nodes we are working with */
	NodeList listIn;

	/* the indirect nodes we are collecting for the next run */
	NodeList listOut;
	nodenum_t* pNodeList[2];

	bitmap_t *listout_bitmap;

	nodenum_t *pGroupNodes;
	count_t groupCount;
	bitmap_t *pBitmapGroup;
	
	EGroupContainsValue groupContainsValue;
};
