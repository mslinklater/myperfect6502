#include "netlistsim.h"
#include <cstring>
#include <cassert>

void NetListSim::SetupNodesAndTransistors(const std::vector<Transistor>& transdefs, const std::vector<bool>& node_is_pullup, nodenum_t vss, nodenum_t vcc)
{
	numNodes = node_is_pullup.size();
	numTransistors = transdefs.size();

	vss = vss;
	vcc = vcc;

	pullupNodes.resize(numNodes);
	pulldownNodes.resize(numNodes);
	nodeState.resize(numNodes);

	nodeGates.resize(numNodes);
	for (count_t i = 0; i < numNodes; i++) 
	{
		nodeGates[i].resize(numNodes);
	}

	nodeGateCount.resize(numNodes);

	nodeC1C2Offset.resize(numNodes + 1);

	nodesDeps.resize(numNodes);
	nodesLeftDeps.resize(numNodes);
	nodesDependant.resize(numNodes);

	for (count_t i = 0; i < numNodes; i++) 
	{
		nodesDependant[i].resize(numNodes);
	}

	nodesLeftDependant.resize(numNodes);

	for (count_t i = 0; i < numNodes; i++) 
	{
		nodesLeftDependant[i].resize(numNodes);
	}

	transistorsGate.resize(numTransistors);
	transistorsC1.resize(numTransistors);
	transistorsC2.resize(numTransistors);
	onTransistors.resize(numTransistors);
	onTransistorsCount.resize(numTransistors);

	// list buffers
	nodeList[0].resize(numNodes);
	nodeList[1].resize(numNodes);

	listoutBitmap.resize(numNodes);

	groupNodes.resize(numNodes);

	groupBitmap.resize(numNodes);

	listIn.pNodes = nodeList[0].data();
    listIn.count = 0;

	listOut.pNodes = nodeList[1].data();
    listOut.count = 0;

	count_t i;
	/* copy nodes into r/w data structure */
	for (i = 0; i < numNodes; i++) 
	{
		pullupNodes[i] = node_is_pullup[i];
		nodeGateCount[i] = 0;
	}

	/* copy transistors into r/w data structure */
	count_t j = 0;

	for (i = 0; i < numTransistors; i++) 
	{
		nodenum_t gate = transdefs[i].gate;
		nodenum_t c1 = transdefs[i].c1;
		nodenum_t c2 = transdefs[i].c2;

		/* skip duplicate transistors */
		bool found = false;

		for (count_t j2 = 0; j2 < j; j2++) 
		{
			if (transistorsGate[j2] == gate &&
				((transistorsC1[j2] == c1 && transistorsC2[j2] == c2) ||
				 (transistorsC1[j2] == c2 && transistorsC2[j2] == c1))) 
			{
				found = true;
			}
		}
		if (!found) 
		{
			transistorsGate[j] = gate;
			transistorsC1[j] = c1;
			transistorsC2[j] = c2;
			j++;
		}
	}
	numTransistors = j;

	/* cross reference transistors in nodes data structures */
	/* start by computing how many c1c2 entries should be created for each node */
//	count_t *c1c2count = new count_t(numNodes);

	std::vector<count_t> c1c2count;
	c1c2count.resize(numNodes);

	count_t c1c2total = 0;

	for (i = 0; i < numTransistors; i++) 
	{
		nodenum_t gate = transistorsGate[i];
		nodenum_t c1 = transistorsC1[i];
		nodenum_t c2 = transistorsC2[i];

		nodenum_t gateCount = nodeGateCount[gate];

		assert(gate < numNodes);
		assert(gateCount < numNodes);

		nodeGates[gate][gateCount] = i;
		nodeGateCount[gate]++;

		assert(c1 < numNodes);
		assert(c2 < numNodes);

		c1c2count[c1]++;
		c1c2count[c2]++;
		c1c2total += 2;
	}
#if 1

	/* then sum the counts to find each node's offset into the c1c2 array */
	count_t c1c2offset = 0;

	for (i = 0; i < numNodes; i++) 
	{
		nodeC1C2Offset[i] = c1c2offset;
		c1c2offset += c1c2count[i];
	}

	nodeC1C2Offset[i] = c1c2offset;
	/* create and fill the nodes_c1c2s array according to these offsets */
	nodeC1C2s.resize(c1c2total);

//	std::memset(c1c2count, 0, numNodes * sizeof(*c1c2count));
	for(int i=0 ; i<c1c2count.size() ; ++i)
	{
		c1c2count[i] = 0;
	}

	for (i = 0; i < numTransistors; i++) 
	{
		nodenum_t c1 = transistorsC1[i];
		nodenum_t c2 = transistorsC2[i];
		nodeC1C2s[nodeC1C2Offset[c1] + c1c2count[c1]++] = NewC1C2(i, c2);
		nodeC1C2s[nodeC1C2Offset[c2] + c1c2count[c2]++] = NewC1C2(i, c1);
	}

//	delete c1c2count;

	for (i = 0; i < numNodes; i++) 
	{
		nodesDeps[i] = 0;
		nodesLeftDeps[i] = 0;
		for (count_t g = 0; g < nodeGateCount[i]; g++) 
		{
			transnum_t t = nodeGates[i][g];
			nodenum_t c1 = transistorsC1[t];
			if (c1 != vss && c1 != vcc) 
			{
				AddNodesDependant(i, c1);
			}
			nodenum_t c2 = transistorsC2[t];
			if (c2 != vss && c2 != vcc) 
			{
				AddNodesDependant(i, c2);
			}
			if (c1 != vss && c1 != vcc) 
			{
				AddNodesLeftDependant(i, c1);
			} 
			else 
			{
				AddNodesLeftDependant(i, c2);
			}
		}
	}
#endif
}

void NetListSim::AddNodesDependant(nodenum_t a, nodenum_t b)
{
	for (count_t g = 0; g < nodesDeps[a]; g++)
	{
		if (nodesDependant[a][g] == b)
		{
			return;
		}
	}

	nodesDependant[a][nodesDeps[a]++] = b;
}

void NetListSim::AddNodesLeftDependant(nodenum_t a, nodenum_t b)
{
	for (count_t g = 0; g < nodesLeftDeps[a]; g++)
	{
		if (nodesLeftDependant[a][g] == b)
		{
			return;
		}
	}

	nodesLeftDependant[a][nodesLeftDeps[a]++] = b;
}

void NetListSim::DestroyNodesAndTransistors()
{

}

void NetListSim::SetNode(nodenum_t nn, bool s)
{

}

bool NetListSim::IsNodeHigh(nodenum_t nn)
{
	return false;
}

unsigned int NetListSim::ReadNodes(int count, nodenum_t *nodelist)
{
	return 0;
}

void NetListSim::WriteNodes(int count, nodenum_t *nodelist, int v) 
{

}

void NetListSim::RecalcNodeList() 
{

}

void NetListSim::StabilizeChip() 
{

}
