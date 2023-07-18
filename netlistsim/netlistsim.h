# pragma once

#include "types.h"

class NetListSim
{
public:
	NetListSim(){}
	virtual ~NetListSim(){}

    void SetupNodesAndTransistors(Transistor *transdefs, bool *node_is_pullup, nodenum_t numNodes, nodenum_t numTransistors, nodenum_t vss, nodenum_t vcc);

    void DestroyNodesAndTransistors();

    void SetNode(nodenum_t nn, bool s);

    bool IsNodeHigh(nodenum_t nn);

    unsigned int ReadNodes(int count, nodenum_t *nodelist);

    void WriteNodes(int count, nodenum_t *nodelist, int v);

    void RecalcNodeList();

    void StabilizeChip();
private:
};
