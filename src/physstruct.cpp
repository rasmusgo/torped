#include "physstruct.hpp"

#include <memory>

#include "alstruct.hpp"
#include "gameapp.hpp"
#include "loadphysics.hpp"
#include "logging.hpp"
#include "physfsstruct.hpp"
#include "texture.hpp"

// lock mutex before access to phyInstances
std::mutex phyInstances_lock;
std::vector<std::unique_ptr<PhyInstance>> phyInstances;

void PrintPhys(PhyInstance *inst, std::ostream *ostr = NULL)
{
    std::stringstream buffer;
    if (ostr == NULL)
        ostr = &buffer;
    /*
    App::console << inst->phys->points_count << " points" << std::endl;
    {
        PhyPoint *it = inst->phys->points;
        PhyPoint *end = inst->phys->points + inst->phys->points_count+inst->phys->nodes_count;
        while (it != end)
        {
            App::console << " pos " << it->pos << " mass " << 1.0/it->inv_mass << std::endl;
            ++it;
        }
    }

    App::console << inst->phys->nodes_count << " nodes" << std::endl;
    {
        PhyNode *it = inst->phys->nodes;
        PhyNode *end = inst->phys->nodes + inst->phys->nodes_count;
        while (it != end)
        {
            App::console << " pos " << it->pos << std::endl;
            ++it;
        }
    }
    */
    *ostr << inst->phys->rigids_count << " rigids" << std::endl;
    {
        PhyRigid *it = inst->phys->rigids;
        PhyRigid *end = inst->phys->rigids + inst->phys->rigids_count;
        while (it != end)
        {
            *ostr << " points " << it->points - inst->phys->points << " nodes " << it->nodes - inst->phys->nodes << std::endl;
            *ostr << " pos " << it->pos << " vel " << it->vel << std::endl;
            *ostr << " rot " << it->orient << " spin " << it->spin << std::endl;
            *ostr << " mass " << it->inv_mass << " inertia " << it->inv_inertia << std::endl;
            ++it;
        }
    }

    *ostr << inst->phys->springs_count << " springs" << std::endl;
    {
        PhySpring *it = inst->phys->springs;
        PhySpring *end = inst->phys->springs + inst->phys->springs_count;
        while (it != end)
        {
            *ostr << " p1 " << it->p1 - inst->phys->points << " p2 " << it->p2 - inst->phys->points;
            *ostr << " k " << it->k << " d " << it->d << " s " << it->s << std::endl;
            ++it;
        }
    }

    *ostr << inst->phys->joints_count << " joints" << std::endl;
    {
        PhyJoint *it = inst->phys->joints;
        PhyJoint *end = inst->phys->joints + inst->phys->joints_count;
        while (it != end)
        {
            *ostr << " p1 " << it->p1 - inst->phys->points << " p2 " << it->p2 - inst->phys->points;
            *ostr << " k " << it->k << " d " << it->d << " s " << it->s << std::endl;
            ++it;
        }
    }
    if (ostr == &buffer)
    {
        LOG_S(INFO) << "Physics:\n" << buffer.str();
    }
}

bool InitPhys()
{
    // Clear errors
    return true;
}

void QuitPhys()
{
    PhyInstance::DeleteAll();
}

PhyInstance::~PhyInstance()
{
    // Destructors should be called here for everything that needs it
    if (phys != nullptr)
        phys->~Physics();

    for (unsigned int i = 0; i < textures_count; ++i)
    {
        textures[i].~Texture();
    }

    for (unsigned int i = 0; i < phys->sounds_count; ++i)
    {
        alDeleteSources(1, &phys->sounds[i].source);
    }
}

/*
// Copy constructor
PhyInstance::PhyInstance(const PhyInstance& other)
{

}

PhyInstance& operator=(const PhyInstance& other); // Copy assignment operator
*/

void PhyInstance::DeleteAll()
{
    std::lock_guard<std::mutex> lock(phyInstances_lock);
    phyInstances.clear();
}

void PhyInstance::Remove()
{
    std::lock_guard<std::mutex> lock(phyInstances_lock);
    // should find the object and remove it...
    for (auto it = phyInstances.begin(); it != phyInstances.end(); ++it)
    {
        if ( it->get() == this )
        {
            phyInstances.erase(it);
            return;
        }
    }
}

template <class T>
T Lerp(const T &a, const T &b, float f)
{
    //f = Clamp(f, 0.0f, 1.0f);
    return a*(1.0f-f) + b*f;
};

// TODO: separate more to allow a service thread
PhyInstance* PhyInstance::InsertPhysXML(const char *filename)
{
    LOG_SCOPE_F(2, "InsertPhysXML");
    std::unique_ptr<PhyInstance> inst = LoadPhysXML(filename);
    if (inst == nullptr || inst->phys == nullptr)
    {
        LOG_S(ERROR) << "LoadPhysXML failed";
        return nullptr;
    }
    // keep track of the physics instance
    // lock mutex before access to phyInstances
    std::lock_guard<std::mutex> lock(phyInstances_lock);
    phyInstances.push_back(std::move(inst));
    return phyInstances.back().get();
}

int PhyInstance::UpdatePhys(const char name[])
{
    if ( poses.find(name) == poses.end() )
        return 0;

    poses[name].Apply(this);
    return 1;
}

int PhyInstance::UpdatePhysBlend(const char name[], float a, float b)
{
    if ( poses.find(name) == poses.end() )
        return 0;

    poses[name].Blend(this, a, b);
    return 1;
}

std::vector<REAL> PhyInstance::PollPhys(const char pollstring[])
{
    TypeName typeName;
    std::stringstream translator;
    std::vector<REAL> ret;
    translator << pollstring;
    translator >> typeName.type;
    // stuff without names
    if (typeName.type=="gravity")
    {
        ret.push_back(phys->gravity.x / (phys->time * phys->time));
        ret.push_back(phys->gravity.y / (phys->time * phys->time));
        ret.push_back(phys->gravity.z / (phys->time * phys->time));
    }
    else if (typeName.type=="timestep")
        ret.push_back(phys->time);
    else if(!(translator >> typeName.name))
    {
        LOG_S(WARNING) << "warning: invalid nameless poll";
        return ret;
    }

    if (typeName.type=="node")
    {
        if ( namesIndex.find(typeName) == namesIndex.end() )
        {
            LOG_S(WARNING) << "warning: could not find node \"" << typeName.name << "\"";
            return ret;
        }
        std::string what;
        translator >> what;
        if (what=="pos")
        {
            ret.push_back(phys->nodes[namesIndex[typeName]].pos.x);
            ret.push_back(phys->nodes[namesIndex[typeName]].pos.y);
            ret.push_back(phys->nodes[namesIndex[typeName]].pos.z);
        }
        else
        {
            LOG_S(WARNING) << "nodes has no matching keyvalue for \"" << what << "\"";
            return ret;
        }
    }
    else if (typeName.type=="point")
    {
        if ( namesIndex.find(typeName) == namesIndex.end() )
        {
            LOG_S(WARNING) << "could not find point \"" << typeName.name << "\"";
            return ret;
        }
        std::string what;
        translator >> what;
        if (what=="pos")
        {
            ret.push_back(phys->points[namesIndex[typeName]].pos.x);
            ret.push_back(phys->points[namesIndex[typeName]].pos.y);
            ret.push_back(phys->points[namesIndex[typeName]].pos.z);
        }
        else if (what=="vel")
        {
            ret.push_back(phys->points[namesIndex[typeName]].vel.x);
            ret.push_back(phys->points[namesIndex[typeName]].vel.y);
            ret.push_back(phys->points[namesIndex[typeName]].vel.z);
        }
        else if (what=="mass")
        {
            ret.push_back(1.0 / phys->points[namesIndex[typeName]].inv_mass);
        }
        else
        {
            LOG_S(WARNING) << "points has no matching keyvalue for \"" << what << "\"";
            return ret;
        }
    }
    else if (typeName.type=="spring")
    {
        if ( namesIndex.find(typeName) == namesIndex.end() )
        {
            LOG_S(WARNING) << "could not find spring \"" << typeName.name << "\"";
            return ret;
        }
        std::string what;
        translator >> what;
        if (what=="length")
            ret.push_back(phys->springs[namesIndex[typeName]].l);
        else if (what=="real_length")
            ret.push_back(phys->springs[namesIndex[typeName]].rl);
        else if (what=="k")
            ret.push_back(phys->springs[namesIndex[typeName]].k);
        else if (what=="d")
            ret.push_back(phys->springs[namesIndex[typeName]].d);
        else if (what=="s")
            ret.push_back(phys->springs[namesIndex[typeName]].s);
        else
        {
            LOG_S(WARNING) << "springs has no matching keyvalue for \"" << what << "\"";
            return ret;
        }
    }
    else if (typeName.type=="joint")
    {
        if ( namesIndex.find(typeName) == namesIndex.end() )
        {
            LOG_S(WARNING) << "could not find joint \"" << typeName.name << "\"";
            return ret;
        }
        std::string what;
        translator >> what;
        if (what=="k")
            ret.push_back(phys->joints[namesIndex[typeName]].k);
        else if (what=="d")
            ret.push_back(phys->joints[namesIndex[typeName]].d);
        else if (what=="s")
            ret.push_back(phys->joints[namesIndex[typeName]].s);
        else
        {
            LOG_S(WARNING) << "joints has no matching keyvalue for \"" << what << "\"";
            return ret;
        }
    }
    else if (typeName.type=="rigid")
    {
        if ( namesIndex.find(typeName) == namesIndex.end() )
        {
            LOG_S(WARNING) << "warning: could not find rigid \"" << typeName.name << "\"";
            return ret;
        }
        std::string what;
        translator >> what;
        if (what=="pos")
        {
            ret.push_back(phys->rigids[namesIndex[typeName]].pos.x);
            ret.push_back(phys->rigids[namesIndex[typeName]].pos.y);
            ret.push_back(phys->rigids[namesIndex[typeName]].pos.z);
        }
        else if (what=="vel")
        {
            ret.push_back(phys->rigids[namesIndex[typeName]].vel.x);
            ret.push_back(phys->rigids[namesIndex[typeName]].vel.y);
            ret.push_back(phys->rigids[namesIndex[typeName]].vel.z);
        }
        else if (what=="mass")
            ret.push_back(1.0 / phys->rigids[namesIndex[typeName]].inv_mass);
        else if (what=="inertia")
        {
            Vec3r tmp = phys->rigids[namesIndex[typeName]].inv_inertia;
            ret.push_back(1.0 / tmp.x);
            ret.push_back(1.0 / tmp.y);
            ret.push_back(1.0 / tmp.z);
        }
        else if (what=="rot" || what=="orientation" || what=="orient")
        {
            ret.push_back(phys->rigids[namesIndex[typeName]].orient.w);
            ret.push_back(phys->rigids[namesIndex[typeName]].orient.vec.x);
            ret.push_back(phys->rigids[namesIndex[typeName]].orient.vec.y);
            ret.push_back(phys->rigids[namesIndex[typeName]].orient.vec.z);
        }
        else if (what=="spin")
        {
            ret.push_back(phys->rigids[namesIndex[typeName]].spin.x);
            ret.push_back(phys->rigids[namesIndex[typeName]].spin.y);
            ret.push_back(phys->rigids[namesIndex[typeName]].spin.z);
        }
        else
        {
            LOG_S(WARNING) << "rigids has no matching keyvalue for \"" << what << "\"";
            return ret;
        }
    }
    return ret;
}

PhyPoint* PhyInstance::FindPoint(std::string name)
{
    TypeName tn;
    tn.name = name;
    tn.type = "point";
    auto it = namesIndex.find(tn);
    if (it != namesIndex.end())
        return phys->points + it->second;

    tn.type = "node";
    it = namesIndex.find(tn);
    if (it != namesIndex.end())
        return phys->points + phys->points_count + it->second;

    LOG_S(WARNING) << "could not find point/node \"" << name << "\"";
    return NULL;
}

int PhyInstance::FindPointIndex(std::string name)
{
    TypeName tn;
    tn.name = name;
    tn.type = "point";
    auto it = namesIndex.find(tn);
    if (it != namesIndex.end())
        return it->second;

    tn.type = "node";
    it = namesIndex.find(tn);
    if (it != namesIndex.end())
        return typeCount["point"] + it->second;

    LOG_S(WARNING) << "could not find point/node \"" << name << "\"";
    return 0;
}

void PhyInstance::RecalculateNormals()
{
    for (Vec3r *it = glnormals, *end = glnormals+glnormals_count; it != end; ++it)
        it->SetToZero();

    Vec3r normal;

    decltype(gltriangle_verts) it = gltriangle_verts;
    decltype(gltriangle_verts) end = gltriangle_verts + gltriangle_verts_count;
    decltype(gltriangle_normals) it2 = gltriangle_normals;

    while (it != end)
    {
        normal = (*it[1] - *it[0]).Cross(*it[2] - *it[0]);

        *(it2[0]) += normal;
        *(it2[1]) += normal;
        *(it2[2]) += normal;

        it += 3;
        it2 += 3;
    }

    it = glquad_verts;
    end = glquad_verts + glquad_verts_count;
    it2 = glquad_normals;

    while (it != end)
    {
        normal = (*it[2] - *it[0]).Cross(*it[3] - *it[1]);

        *(it2[0]) += normal;
        *(it2[1]) += normal;
        *(it2[2]) += normal;
        *(it2[3]) += normal;

        it += 4;
        it2 += 4;
    }
}

void PhyInstance::CrashHandling()
{
    if (phys->insane)
    {
        // Load
        LOG_S(WARNING) << "CrashHandling load: insane: " << phys->insane << "...";
        char * profiler_buffer = new char[sizeof(Profiler)];
        memcpy(profiler_buffer, &phys->profiler, sizeof(Profiler));
        phys->insane = 0;
        memcpy(memPool.data(), memPool2.data(), memPool.size());
        memcpy(&phys->profiler, profiler_buffer, sizeof(Profiler));
        //LOG("CrashHandling early end0 phys->insane: " << phys->insane);
    }
    else
    {
        // Save
        //LOG("CrashHandling save...");
        memcpy(memPool2.data(), memPool.data(), memPool.size());
        //LOG("CrashHandling end");
    }
}
