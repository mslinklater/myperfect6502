#pragma once

#include <cstdint>
#include <vector>

typedef uint16_t nodenum_t;

/* the smallest types to fit the numbers */
typedef uint16_t transnum_t;
typedef uint16_t count_t;
/* nodenum_t is declared in types.h, because it's API */

typedef struct {
	int gate;
	int c1;
	int c2;
} Transistor;

/* list of nodes that need to be recalculated */
typedef struct {
	nodenum_t *pNodes;
	count_t count;
} NodeList;

/* a transistor from the point of view of one of the connected nodes */
typedef struct {
	transnum_t transistor;
	nodenum_t other_node;
} C1C2;

