# pragma once

#include "types.h"

class NetListSim
{
public:
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
		nodenum_t gate;
		nodenum_t c1;
		nodenum_t c2;
	} Transistor;

	typedef struct {
		nodenum_t *pNodes;
		count_t count;
	} NodeList;

	/* a transistor from the point of view of one of the connected nodes */
	typedef struct {
		transnum_t transistor;
		nodenum_t other_node;
	} c1c2_t;

	NetListSim(){}
	virtual ~NetListSim(){}

    void SetupNodesAndTransistors(const std::vector<Transistor>& transdefs, const std::vector<bool>& node_is_pullup, nodenum_t numNodes, nodenum_t numTransistors, nodenum_t vss, nodenum_t vcc)
	{}

    void DestroyNodesAndTransistors()
	{}

    void SetNode(nodenum_t nn, bool s)
	{}

    bool IsNodeHigh(nodenum_t nn)
	{
		return false;
	}

    unsigned int ReadNodes(int count, nodenum_t *nodelist)
	{
		return 0;
	}

    void WriteNodes(int count, nodenum_t *nodelist, int v) {}

    void RecalcNodeList() {}

    void StabilizeChip() {}
private:

};
