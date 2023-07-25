#include "debug.h"

void Debug::AddBitArrayToStateString(std::string& str, const std::vector<bool>& array)
{
	for(bool node : array)
	{
		if (node)
		{
			str.append("1");
		}
		else
		{
			str.append("0");
		}
	}
}

void Debug::AddNodeNumArrayToStateString(std::string& str, const std::vector<nodenum_t>& array)
{
	for(nodenum_t i : array)
	{
		str.append(std::to_string(i));
	}
}

bool Debug::CheckCores(state_t* pOldCore, const NetListSim& newCore)
{
	return true;
}
