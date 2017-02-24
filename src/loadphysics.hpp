#include <memory>

#include "physstruct.hpp"

std::unique_ptr<PhyInstance> LoadPhysXML(const char *filename);
void ParsePhysXML(PhyInstance *inst, TiXmlHandle *hRoot);
