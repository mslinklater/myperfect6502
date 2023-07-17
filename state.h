#pragma once

#include <vector>

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
	std::vector<bool> pullupNodes;
	std::vector<bool> pulldownNodes;
	std::vector<bool> nodeState;

	nodenum_t **ppNodeGates;	// num-nodes size array of pointers... each one points to num-nodes array of shorts

	std::vector<c1c2_t> nodeC1C2s;

	std::vector<count_t> nodeGateCount;

	// Offsets in to the C1C2s array
	std::vector<count_t> nodeC1C2Offset;

	nodenum_t *nodes_dependants;
	nodenum_t *nodes_left_dependants;

	nodenum_t **nodes_dependant;
	nodenum_t **nodes_left_dependant;

	/* everything that describes a transistor */
	std::vector<nodenum_t> transistorsGate;
	std::vector<nodenum_t> transistorsC1;
	std::vector<nodenum_t> transistorsC2;

	std::vector<bool> onTransistors;

	// QUESTION - Can we have more list to multi-thread ?

	/* the nodes we are working with */
	NodeList listIn;
	NodeList listOut;

	/* the indirect nodes we are collecting for the next run */
	nodenum_t* pNodeList[2];

	bitmap_t *listout_bitmap;

	std::vector<nodenum_t> groupNodes;
//	nodenum_t *pGroupNodes;
	count_t groupCount;

	bitmap_t *pBitmapGroup;
//	std::vector<bool> groupBitmap;	// why does this cause it to fail ?
	
	EGroupContainsValue groupContainsValue;
};
