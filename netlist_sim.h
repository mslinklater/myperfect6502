#include "netlistsim.h"

struct state_t;

// The public API to the netlist simulation

state_t* SetupNodesAndTransistors(std::vector<Transistor>& transdefs, std::vector<bool>& node_is_pullup, nodenum_t vss, nodenum_t vcc);

void DestroyNodesAndTransistors(state_t *state);

void SetNode(state_t *state, nodenum_t nn, bool s);

bool IsNodeHigh(state_t *state, nodenum_t nn);

unsigned int ReadNodes(state_t *state, int count, nodenum_t *nodelist);

void WriteNodes(state_t *state, int count, nodenum_t *nodelist, int v);

void RecalcNodeList(state_t *state);

void StabilizeChip(state_t *state);

std::string GetStateString(state_t *state);
