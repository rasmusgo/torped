#include <memory>

#include "json.hpp"
#include "physstruct.hpp"

std::unique_ptr<PhyInstance> LoadPhysXML(const char *filename);
void ParsePhysXML(PhyInstance *inst, TiXmlHandle *hRoot);

std::unique_ptr<PhyInstance> LoadPhysJSON(const char *filename);
void ParsePhysJSON(PhyInstance *inst, const Json &hRoot);
