#pragma once

#include "netliststate.h"

namespace Debug
{
	bool CheckCores(state_t* pOldCore, const NetListSim& newCore);
	
	void AddBitArrayToStateString(std::string& str, const std::vector<bool>& array);
	void AddNodeNumArrayToStateString(std::string& str, const std::vector<nodenum_t>& array);
}