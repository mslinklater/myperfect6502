#ifndef INCLUDED_FROM_NETLIST_SIM_C
#define state_t void
#endif

// The public API to the netlist simulation

state_t* setupNodesAndTransistors(Transistor *transdefs, BOOL *node_is_pullup, nodenum_t numNodes, nodenum_t numTransistors, nodenum_t vss, nodenum_t vcc);

void DestroyNodesAndTransistors(state_t *state);

void setNode(state_t *state, nodenum_t nn, BOOL s);

BOOL isNodeHigh(state_t *state, nodenum_t nn);

unsigned int readNodes(state_t *state, int count, nodenum_t *nodelist);

void writeNodes(state_t *state, int count, nodenum_t *nodelist, int v);

void recalcNodeList(state_t *state);

void stabilizeChip(state_t *state);
