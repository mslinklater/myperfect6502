#pragma once

#include <vector>
#include <set>
#include "netlistsim.h"

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

class state_t {
public:
	nodenum_t numNodes;
	nodenum_t numTransistors;
	nodenum_t vss;
	nodenum_t vcc;

	/* everything that describes a node */
	std::vector<bool> pullupNodes;
	std::vector<bool> pulldownNodes;
	std::vector<bool> nodeState;

	std::vector<std::vector<nodenum_t>> nodeGates;

	std::vector<C1C2> nodeC1C2s;

	std::vector<count_t> nodeGateCount;

	// Offsets in to the C1C2s array
	std::vector<count_t> nodeC1C2Offset;

	std::vector<nodenum_t> nodesDeps;
	std::vector<nodenum_t> nodesLeftDeps;

	std::vector<std::vector<nodenum_t>> nodesDependant;
	std::vector<std::vector<nodenum_t>> nodesLeftDependant;

	/* everything that describes a transistor */
	std::vector<nodenum_t> transistorsGate;
	std::vector<nodenum_t> transistorsC1;
	std::vector<nodenum_t> transistorsC2;

	std::vector<bool> onTransistors;
	std::vector<int> onTransistorsCount;

	/* the nodes we are working with */
	NodeList listIn;
	NodeList listOut;

	/* the indirect nodes we are collecting for the next run */
	std::vector<nodenum_t> nodeList[2];

	std::vector<nodenum_t> groupNodes;
	count_t groupCount;

	std::vector<bool>listoutBitmap;
	std::vector<bool>groupBitmap;
	
	EGroupContainsValue groupContainsValue;

	int maxGroupCount;

	state_t()
		: groupCount(0)
	{}

	virtual ~state_t(){}

};
