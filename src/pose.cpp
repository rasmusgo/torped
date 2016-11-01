#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "physstruct.h"
#include "pose.h"
#include "physics.h"
#include "parser.h"
#include "console.h"

using namespace std;

void Pose::Load(const TiXmlHandle &hPose, PhyInstance *inst)
{
    TypeName tn;
    TiXmlElement *pElem, *mesh;
    tn.type = "spring";
    mesh = hPose.FirstChild("mesh").Element();
    for (; mesh; mesh = mesh->NextSiblingElement("mesh"))
    {
        pElem = TiXmlHandle(mesh).FirstChild("spring").Element();
        for (; pElem; pElem = pElem->NextSiblingElement("spring"))
        {
            SpringState spring = {NAN, NAN, NAN, NAN};

            stringstream ss;
            ss << pElem->Attribute("name");
            ss >> tn.name;
            ss.clear();
            ss << pElem->Attribute("k");
            ss >> spring.k;
            ss.clear();
            ss << pElem->Attribute("d");
            ss >> spring.d;
            ss.clear();
            ss << pElem->Attribute("s");
            ss >> spring.s;
            ss.clear();
            ss << pElem->Attribute("length");
            ss >> spring.l;

            spring.k *= inst->phys->time*inst->phys->time;
            spring.d *= inst->phys->time;
            spring.s *= inst->phys->time*inst->phys->time;

            springStates[ inst->namesIndex[tn] ] = spring;
        }
    }
}

template <class T>
static inline void BlendFunc(T &dest, T arg, float a, float b)
{
    if (arg == arg)
        dest = dest*a + arg*b;
}

void Pose::Apply(PhyInstance *inst)
{
    typeof(springStates.end()) it;
    for (it = springStates.begin(); it != springStates.end(); ++it)
    {
        PhySpring *spring = &inst->phys->springs[it->first];
        BlendFunc(spring->l, it->second.l, 0, 1);
        BlendFunc(spring->k, it->second.k, 0, 1);
        BlendFunc(spring->d, it->second.d, 0, 1);
        BlendFunc(spring->s, it->second.s, 0, 1);
    }
}

void Pose::Blend(PhyInstance *inst, float a, float b)
{
    typeof(springStates.end()) it;
    for (it = springStates.begin(); it != springStates.end(); ++it)
    {
        PhySpring *spring = &inst->phys->springs[it->first];
        BlendFunc(spring->l, it->second.l, a, b);
        BlendFunc(spring->k, it->second.k, a, b);
        BlendFunc(spring->d, it->second.d, a, b);
        BlendFunc(spring->s, it->second.s, a, b);
    }
}
