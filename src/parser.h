#ifndef PARSER_H
#define PARSER_H

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <vector>
#include <string>
#include <map>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

struct TypeName
{
	std::string type, name;
	bool operator < (const TypeName &other) const
	{
		if (type < other.type)
			return true;
		if (type > other.type)
			return false;
		if (name < other.name)
			return true;
		return false;
	}
};

class Parser
{
public:
	std::vector<std::string> lines;
	std::map<TypeName, int> namesIndex;
	std::map<std::string, int> typeCount;
	int Load(const char *filename);
};

#endif //PARSER_H
