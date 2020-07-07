#include "logging.hpp"
#include "parser.hpp"
#include "physics.hpp"
#include "physstruct.hpp"
#include "pose.hpp"

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
            if (auto name = pElem->Attribute("name"))
            {
                ss << name;
                ss >> tn.name;
                ss.clear();
            }
            if (auto k = pElem->Attribute("k"))
            {
                ss << k;
                ss >> spring.k;
                ss.clear();
            }
            if (auto d = pElem->Attribute("d"))
            {
                ss << d;
                ss >> spring.d;
                ss.clear();
            }
            if (auto s = pElem->Attribute("s"))
            {
                ss << s;
                ss >> spring.s;
                ss.clear();
            }
            if (auto length = pElem->Attribute("length"))
            {
                ss << length;
                ss >> spring.l;
                ss.clear();
            }

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
    for (auto it = springStates.begin(); it != springStates.end(); ++it)
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
    for (auto it = springStates.begin(); it != springStates.end(); ++it)
    {
        PhySpring *spring = &inst->phys->springs[it->first];
        BlendFunc(spring->l, it->second.l, a, b);
        BlendFunc(spring->k, it->second.k, a, b);
        BlendFunc(spring->d, it->second.d, a, b);
        BlendFunc(spring->s, it->second.s, a, b);
    }
}
