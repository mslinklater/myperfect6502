# pragma once

#include "types.h"
#include <map>

/*
Terminology:

Transistor - as you would expect
Node - an island of electrical potential... a wire... nodes are what connect transistors
Group - A node state... what the electrical properties are

*/

class NetListSim
{
public:

	// Some types and simple structures used internally

	enum EGroup {
		kNothing,
		kHigh,
		kPullup,
		kPulldown,
		kVcc,
		kVss
	};

	typedef uint16_t nodenum_t;
	typedef uint16_t transnum_t;
	typedef uint16_t count_t;

	typedef struct {
		nodenum_t gate;		// out
		nodenum_t c1;		// source ?
		nodenum_t c2;		// drain ?
	} Transistor;

	typedef struct {
		nodenum_t *pNodes;
		count_t count;
	} NodeList;

	/* a transistor from the point of view of one of the connected nodes */
	typedef struct {
		transnum_t transistor;
		nodenum_t other_node;
	} C1C2;

	static inline C1C2
	NewC1C2(transnum_t tn, nodenum_t n)
	{
		C1C2 c = { tn, n };
		return c;
	}

	// Lifecycle

	NetListSim(){}
	virtual ~NetListSim(){}

	// The public API - call this stuff from your own code... it should do everything you need

    void SetupNodesAndTransistors(const std::vector<Transistor>& transdefs, const std::vector<bool>& node_is_pullup, nodenum_t numNodes, nodenum_t numTransistors, nodenum_t vss, nodenum_t vcc);

    void DestroyNodesAndTransistors();
    void SetNode(nodenum_t nn, bool s);
    bool IsNodeHigh(nodenum_t nn);
    unsigned int ReadNodes(int count, nodenum_t *nodelist);
    void WriteNodes(int count, nodenum_t *nodelist, int v);
    void RecalcNodeList();
    void StabilizeChip();

private:

	// Internal methods

	void AddNodesDependant(nodenum_t a, nodenum_t b);

	void AddNodesLeftDependant(nodenum_t a, nodenum_t b);

private:
	nodenum_t	numNodes;
	nodenum_t	numTransistors;

	nodenum_t	vss;	// Ground... 0V
	nodenum_t	vcc;	// +V

	// Bit array of nodes which are pull-up. This is initialised from the input data
	std::vector<bool> pullupNodes;

	// Bit array of nodes which have been pulled down temporarily
	std::vector<bool> pulldownNodes;

	// Bit array of the actual charge state of a node
	std::vector<bool> nodeState;

	// 2D matrix of the node gates... need to get the structure of this
	std::vector<std::vector<nodenum_t>> nodeGates;

	// The two inputs (collector and base) of a transistor
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

	/* the nodes we are working with */
	NodeList listIn;
	NodeList listOut;

	/* the indirect nodes we are collecting for the next run */
	std::vector<nodenum_t> nodeList[2];

	std::vector<nodenum_t> groupNodes;
	count_t groupCount;

	std::vector<bool>listoutBitmap;
	std::vector<bool>groupBitmap;
	
	EGroup groupContainsValue;

	// Debug/analysis stuff

	std::vector<int> onTransistorsCount;
	std::map<std::string, nodenum_t> nameNodeMap;
};
