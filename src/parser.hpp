#pragma once

#include <vector>
#include <string>
#include <map>

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
