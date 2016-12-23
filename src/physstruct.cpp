#include <fstream>
#include <memory>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "alstruct.h"
#include "gameapp.h"
#include "logging.h"
#include "physfsstruct.h"
#include "physstruct.h"
#include "texture.h"

using namespace std;

// lock mutex before access to phyInstances
SDL_mutex *phyInstances_lock = NULL;
vector<PhyInstance> phyInstances;

void PrintPhys(PhyInstance *inst, ostream *ostr = NULL)
{
    stringstream buffer;
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
    phyInstances_lock = SDL_CreateMutex();
    // Clear errors
    return true;
}

void QuitPhys()
{
    PhyInstance::DeleteAll();
    SDL_DestroyMutex(phyInstances_lock);
    phyInstances_lock = NULL;
}

void PhyInstance::DeleteAll()
{
    SDL_LockMutex(phyInstances_lock);
    for (typeof(phyInstances.begin()) it = phyInstances.begin(); it != phyInstances.end(); ++it)
    {
        // Destructors should be called here for everything that needs it
        it->phys->~Physics();

        for (unsigned int i = 0; i < it->textures_count; ++i)
        {
            it->textures[i].~Texture();
        }

        for (unsigned int i = 0; i < it->phys->sounds_count; ++i)
        {
            alDeleteSources(1, &it->phys->sounds[i].source);
        }

        delete [] it->memPool;
        it->memPool = NULL;
        it->memPool2 = NULL;
    }
    phyInstances.clear();
    SDL_UnlockMutex(phyInstances_lock);
}

void PhyInstance::Remove()
{
    SDL_LockMutex(phyInstances_lock);
    // should find the object and remove it...
    for (typeof(phyInstances.begin()) it = phyInstances.begin(); it != phyInstances.end(); ++it)
    {
        if ( &(*it) == this )
        {
            // Destructors should be called here for everything that needs it
            it->phys->~Physics();

            for (unsigned int i = 0; i < it->textures_count; ++i)
            {
                it->textures[i].~Texture();
            }

            for (unsigned int i = 0; i < it->phys->sounds_count; ++i)
            {
                alDeleteSources(1, &it->phys->sounds[i].source);
            }

            delete [] it->memPool;
            it->memPool = NULL;
            it->memPool2 = NULL;
            phyInstances.erase(it);
            break;
        }
    }
    SDL_UnlockMutex(phyInstances_lock);
}

template <class T>
inline T* AddAndIncrement(char * &place, const int count)
{
    if (count == 0)
        return NULL;
    //    return (T*)(place);

    T *ret;
    ret = new (place) T[count];

    //App::console << __FILE__ << ":" << __LINE__ << " in function " << __FUNCTION__
    //    << " place: " << hex << int((void*)place) << " ret: " << int(ret) << dec << std::endl;

    place += sizeof(T) * count;
    return ret;
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
    PhyInstance inst = PhyInstance::LoadPhysXML(filename);
    if (inst.phys == NULL)
    {
        LOG_S(ERROR) << "LoadPhysXML failed";
        return NULL;
    }
    // keep track of the physics instance
    // lock mutex before access to phyInstances
    SDL_LockMutex(phyInstances_lock);
    // TODO: store inst on the heap and avoid copying
    phyInstances.push_back(inst);
    SDL_UnlockMutex(phyInstances_lock);

    //delete [] filename;
    //filename = NULL;
    VLOG_S(3) << "End of InsertPhysXML";

    return &phyInstances.back();
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
    stringstream translator;
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
        string what;
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
        string what;
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
        string what;
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
        string what;
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
            LOG_S(WARNING) << "warning: could not find rigid \"" << typeName.name << "\"" << endl;
            return ret;
        }
        string what;
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

PhyPoint* PhyInstance::FindPoint(string name)
{
    TypeName tn;
    typeof(namesIndex.end()) it;

    tn.name = name;
    tn.type = "point";
    it = namesIndex.find(tn);
    if (it != namesIndex.end())
        return phys->points + it->second;

    tn.type = "node";
    it = namesIndex.find(tn);
    if (it != namesIndex.end())
        return phys->points + phys->points_count + it->second;

    LOG_S(WARNING) << "could not find point/node \"" << name << "\"";
    return NULL;
}

int PhyInstance::FindPointIndex(string name)
{
    TypeName tn;
    typeof(namesIndex.end()) it;

    tn.name = name;
    tn.type = "point";
    it = namesIndex.find(tn);
    if (it != namesIndex.end())
        return it->second;

    tn.type = "node";
    it = namesIndex.find(tn);
    if (it != namesIndex.end())
        return typeCount["point"] + it->second;

    LOG_S(WARNING) << "could not find point/node \"" << name << "\"";
    return 0;
}

PhyInstance PhyInstance::LoadPhysXML(const char *filename)
{
    //LOG(__FUNCTION__ << ": " << __LINE__);

    PhyInstance inst;
    inst.memPool = NULL;
    inst.memPool2 = NULL;
    inst.memPool_size = 0;
    inst.phys = NULL;

    char* buffer = NULL;
    PhysFSLoadFile(filename, buffer);
    CHECK_NOTNULL_F(buffer, "Failed to open '%s'", filename);
    std::unique_ptr<char> file_buffer_pointer{buffer};

    TiXmlDocument doc(filename);
	if ( !doc.Parse(buffer) )
	{
        LOG_S(ERROR) << filename << ":" << doc.ErrorRow() << "," << doc.ErrorCol() << ": " << doc.ErrorDesc();
        return inst;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement *pElem, *pElem2, *mesh;
	TiXmlHandle hRoot(NULL);

    pElem=hDoc.FirstChildElement().Element();
    // should always have a valid root but handle gracefully if it does
    if (!pElem)
    {
        LOG_S(WARNING) << filename << ": Missing valid root";
        return inst;
    }

    // save this for later
    hRoot=TiXmlHandle(pElem);

    TypeName typeName;
    TypeName tn;

	// Count points
    typeName.type = "point";
    mesh = hRoot.FirstChild("mesh").Element();
    for (; mesh; mesh = mesh->NextSiblingElement("mesh"))
    {
        pElem = TiXmlHandle(mesh).FirstChild(typeName.type).Element();
        for (; pElem; pElem = pElem->NextSiblingElement("point"))
        {
            typeName.name = pElem->Attribute("name");
            if ( typeName.name != "" )
                inst.namesIndex[typeName] = inst.typeCount[typeName.type];
            inst.typeCount[typeName.type]++;
        }
    }

	// Count rigids and nodes
    typeName.type = "rigid";
    pElem = hRoot.FirstChild("rigid").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("rigid"))
    {
        typeName.name = pElem->Attribute("name");
        if ( typeName.name != "" )
            inst.namesIndex[typeName] = inst.typeCount[typeName.type];
        inst.typeCount[typeName.type]++;

        TypeName tn;
        tn.type = "node";
        pElem2 = TiXmlHandle(pElem).FirstChild("node").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("node"))
        {
            tn.name = pElem2->Attribute("name");
            if ( tn.name != "" )
                inst.namesIndex[tn] = inst.typeCount[tn.type];
            inst.typeCount[tn.type]++;
        }
    }

	// Count springs
    typeName.type = "spring";
    mesh = hRoot.FirstChild("mesh").Element();
    for (; mesh; mesh = mesh->NextSiblingElement("mesh"))
    {
        pElem = TiXmlHandle(mesh).FirstChild(typeName.type).Element();
        for (; pElem; pElem = pElem->NextSiblingElement("spring"))
        {
            typeName.name = pElem->Attribute("name");
            if ( typeName.name != "" )
                inst.namesIndex[typeName] = inst.typeCount[typeName.type];
            inst.typeCount[typeName.type]++;
        }
    }

	// Count joints
    typeName.type = "joint";
    mesh = hRoot.FirstChild("joints").Element();
    for (; mesh; mesh = mesh->NextSiblingElement("joints"))
    {
        pElem = TiXmlHandle(mesh).FirstChild(typeName.type).Element();
        for (; pElem; pElem = pElem->NextSiblingElement("joint"))
        {
            typeName.name = pElem->Attribute("name");
            if ( typeName.name != "" )
                inst.namesIndex[typeName] = inst.typeCount["joint"];
            inst.typeCount["joint"]++;
        }
    }

	// Count balloons and their triangles (points)
    typeName.type = "balloon";
    pElem = hRoot.FirstChild("balloon").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("balloon"))
    {
        typeName.name = pElem->Attribute("name");
        if ( typeName.name != "" )
            inst.namesIndex[typeName] = inst.typeCount[typeName.type];
        inst.typeCount[typeName.type]++;

        TypeName tn;
        tn.type = "ppoint";
        pElem2 = TiXmlHandle(pElem).FirstChild("point").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("point"))
        {
            inst.typeCount[tn.type]++;
        }
    }

	// Count motors
    typeName.type = "motor";
    pElem = hRoot.FirstChild("motor").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("motor"))
    {
        typeName.name = pElem->Attribute("name");
        if ( typeName.name != "" )
            inst.namesIndex[typeName] = inst.typeCount[typeName.type];
        inst.typeCount[typeName.type]++;
    }

	// Count sounds
    typeName.type = "sound";
    pElem = hRoot.FirstChild("sound").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("sound"))
    {
        typeName.name = pElem->Attribute("name");
        if ( typeName.name != "" )
            inst.namesIndex[typeName] = inst.typeCount[typeName.type];
        inst.typeCount[typeName.type]++;
    }

    // Count textures
    typeName.type = "texture";
    pElem = hRoot.FirstChild("texture").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("texture"))
    {
        typeName.name = pElem->Attribute("name");
        if ( typeName.name != "" )
        {
            inst.namesIndex[typeName] = inst.typeCount["texture"];
            inst.typeCount["texture"]++;
        }
    }

    // Count renderpasses, triangles, quads and normals
    pElem = hRoot.FirstChild("render").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("render"))
    {
        inst.typeCount["render"]++;

        pElem2 = TiXmlHandle(pElem).FirstChild("gltriangle").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("gltriangle"))
        {
            inst.typeCount["gltriangle"]++;
            int n1 = 0, n2 = 0, n3 = 0;

            if (pElem2->Attribute("normals") != NULL)
            {
                stringstream ss;
                ss << pElem2->Attribute("normals");
                ss >> n1 >> n2 >> n3;
            }
            else
            {
                n1 = inst.FindPointIndex(pElem2->Attribute("p1"));
                n2 = inst.FindPointIndex(pElem2->Attribute("p2"));
                n3 = inst.FindPointIndex(pElem2->Attribute("p3"));
            }

            if (inst.typeCount["glnormal"] <= n1)
                inst.typeCount["glnormal"] = n1+1;
            if (inst.typeCount["glnormal"] <= n2)
                inst.typeCount["glnormal"] = n2+1;
            if (inst.typeCount["glnormal"] <= n3)
                inst.typeCount["glnormal"] = n3+1;
        }

        pElem2 = TiXmlHandle(pElem).FirstChild("glquad").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("glquad"))
        {
            inst.typeCount["glquad"]++;

            int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
            if (pElem2->Attribute("normals") != NULL)
            {
                stringstream ss;
                ss << pElem2->Attribute("normals");
                ss >> n1 >> n2 >> n3 >> n4;
            }
            else
            {
                n1 = inst.FindPointIndex(pElem2->Attribute("p1"));
                n2 = inst.FindPointIndex(pElem2->Attribute("p2"));
                n3 = inst.FindPointIndex(pElem2->Attribute("p3"));
                n4 = inst.FindPointIndex(pElem2->Attribute("p4"));
            }

            if (inst.typeCount["glnormal"] <= n1)
                inst.typeCount["glnormal"] = n1+1;
            if (inst.typeCount["glnormal"] <= n2)
                inst.typeCount["glnormal"] = n2+1;
            if (inst.typeCount["glnormal"] <= n3)
                inst.typeCount["glnormal"] = n3+1;
            if (inst.typeCount["glnormal"] <= n4)
                inst.typeCount["glnormal"] = n4+1;
        }
    }

    // calculate required storage size
    unsigned int size;
    size = sizeof(Physics) +
           sizeof(PhyPoint)    * inst.typeCount["point"]      +
           sizeof(PhyPoint)    * inst.typeCount["node"]       +
           sizeof(PhyNode)     * inst.typeCount["node"]       +
           sizeof(PhySpring)   * inst.typeCount["spring"]     +
           sizeof(PhyJoint)    * inst.typeCount["joint"]      +
           sizeof(PhyRigid)    * inst.typeCount["rigid"]      +
           sizeof(PhyBalloon)  * inst.typeCount["balloon"]    +
           sizeof(PhyPoint*)   * inst.typeCount["ppoint"]     +
           sizeof(PhyMotor)    * inst.typeCount["motor"]      +
           sizeof(PhySound)    * inst.typeCount["sound"]      +
           // vertex and normal pointers
           sizeof(Vec3r*) * 6  * inst.typeCount["gltriangle"] +
           sizeof(Vec3r*) * 8  * inst.typeCount["glquad"]     +
           // gltexcoords
           sizeof(GLfloat) * 6 * inst.typeCount["gltriangle"] +
           sizeof(GLfloat) * 8 * inst.typeCount["glquad"]     +
           // glnormals
           sizeof(Vec3r)       * inst.typeCount["glnormal"]   +
           sizeof(Texture)     * inst.typeCount["texture"]    +
           sizeof(RenderPass)  * inst.typeCount["render"]     +
           0;

    // allocate memory for all the physics stuff
    // double the size to keep a copy of memPool to allow crash replay
    inst.memPool = new (nothrow) char[size*2];
    inst.memPool2 = inst.memPool+size; //new (nothrow) char[size];
    inst.memPool_size = size;

    if (inst.memPool == NULL)
    {
        LOG_S(ERROR) << "Unable to allocate memory: " << size << " bytes";
        return inst;
    }

    memset(inst.memPool, 0, size);

    // insertion place in memory
    char * place = inst.memPool;

    // create the physics instance
    inst.phys = AddAndIncrement<Physics>(place, 1);

    // create points and stuff
    // add points controlled by nodes after regular points
    inst.phys->points           = AddAndIncrement<PhyPoint   >(place, inst.typeCount["point"]+inst.typeCount["node"]);
    inst.phys->nodes            = AddAndIncrement<PhyNode    >(place, inst.typeCount["node"]);
    inst.phys->springs          = AddAndIncrement<PhySpring  >(place, inst.typeCount["spring"]);
    inst.phys->joints           = AddAndIncrement<PhyJoint   >(place, inst.typeCount["joint"]);
    inst.phys->rigids           = AddAndIncrement<PhyRigid   >(place, inst.typeCount["rigid"]);
    inst.phys->balloons         = AddAndIncrement<PhyBalloon >(place, inst.typeCount["balloon"]);
    inst.phys->ppoints          = AddAndIncrement<PhyPoint*  >(place, inst.typeCount["ppoint"]);
    inst.phys->motors           = AddAndIncrement<PhyMotor   >(place, inst.typeCount["motor"]);
    inst.phys->sounds           = AddAndIncrement<PhySound   >(place, inst.typeCount["sound"]);
    inst.gltriangle_verts       = AddAndIncrement<Vec3r*     >(place, inst.typeCount["gltriangle"]*3);
    inst.gltriangle_normals     = AddAndIncrement<Vec3r*     >(place, inst.typeCount["gltriangle"]*3);
    inst.glquad_verts           = AddAndIncrement<Vec3r*     >(place, inst.typeCount["glquad"]*4);
    inst.glquad_normals         = AddAndIncrement<Vec3r*     >(place, inst.typeCount["glquad"]*4);
    inst.gltexcoords            = AddAndIncrement<GLfloat    >(place, inst.typeCount["gltriangle"]*6+inst.typeCount["glquad"]*8);
    inst.glnormals              = AddAndIncrement<Vec3r      >(place, inst.typeCount["glnormal"]);
    inst.textures               = AddAndIncrement<Texture    >(place, inst.typeCount["texture"]);
    inst.renderpasses           = AddAndIncrement<RenderPass >(place, inst.typeCount["render"]);

    // remember count
    inst.phys->points_count     = inst.typeCount["point"];
    inst.phys->nodes_count      = inst.typeCount["node"];
    inst.phys->springs_count    = inst.typeCount["spring"];
    inst.phys->joints_count     = inst.typeCount["joint"];
    inst.phys->rigids_count     = inst.typeCount["rigid"];
    inst.phys->balloons_count   = inst.typeCount["balloon"];
    inst.phys->ppoints_count    = inst.typeCount["ppoint"];
    inst.phys->motors_count     = inst.typeCount["motor"];
    inst.phys->sounds_count     = inst.typeCount["sound"];
    inst.gltriangle_verts_count = inst.typeCount["gltriangle"]*3;
    inst.glquad_verts_count     = inst.typeCount["glquad"]*4;
    inst.glnormals_count        = inst.typeCount["glnormal"];
    inst.textures_count         = inst.typeCount["texture"];
    inst.renderpasses_count     = inst.typeCount["render"];

    inst.ParsePhysXML(&hRoot);

    //PrintPhys(&inst, &ofstream("loadphysxml.txt"));

    VLOG_S(3) << "End of LoadPhysXML";
    return inst; // success
}

void PhyInstance::ParsePhysXML(TiXmlHandle *hRoot)
{
    {
        stringstream ss;
        Vec3r tmp = phys->gravity / phys->time * phys->time;
        ss << hRoot->Element()->Attribute("gravity");
        ss >> tmp;
        tmp *= phys->time * phys->time;
        phys->gravity = tmp;
    }

	TiXmlElement *pElem, *pElem2, *mesh;

	// Init points
	PhyPoint *point = phys->points;
    mesh = hRoot->FirstChild("mesh").Element();
    for (; mesh; mesh = mesh->NextSiblingElement("mesh"))
    {
        pElem = TiXmlHandle(mesh).FirstChild("point").Element();
        for (; pElem; pElem = pElem->NextSiblingElement("point"))
        {
            stringstream ss;
            ss << pElem->Attribute("pos");
            ss >> point->pos;
            ss.clear();
            // TODO: premuliply with timestep
            ss << pElem->Attribute("vel");
            ss >> point->vel;
            ss.clear();
            // TODO: premuliply with timestep
            ss << pElem->Attribute("force");
            ss >> point->force;
            REAL mass = 0;
            ss.clear();
            ss << pElem->Attribute("mass");
            ss >> mass;
            if (mass != 0)
                point->inv_mass = 1.0/mass;
            ++point;
        }
    }

	// Init rigids and nodes
	PhyRigid *rigid = phys->rigids;
	PhyNode *node = phys->nodes;
    pElem = hRoot->FirstChild("rigid").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("rigid"))
    {
        rigid->nodes = node;
        rigid->points = point;

        stringstream ss;
        ss << pElem->Attribute("pos");
        ss >> rigid->pos;
        ss.clear();
        // TODO: premuliply with timestep
        ss << pElem->Attribute("vel");
        ss >> rigid->vel;
        ss.clear();
        // TODO: premuliply with timestep
        ss << pElem->Attribute("force");
        ss >> rigid->force;
        ss.clear();
        rigid->orient = Quat4r(1,0,0,0);
        ss << pElem->Attribute("rot");
        ss >> rigid->orient;
        ss.clear();
        // TODO: premuliply with timestep
        ss << pElem->Attribute("spin");
        ss >> rigid->spin;
        ss.clear();
        // TODO: premuliply with timestep
        ss << pElem->Attribute("torque");
        ss >> rigid->torque;
        REAL mass = 0;
        ss.clear();
        ss << pElem->Attribute("mass");
        ss >> mass;
        if (mass != 0)
            rigid->inv_mass = 1.0/mass;
        Vec3r inertia = Vec3r(0,0,0);
        ss.clear();
        ss << pElem->Attribute("inertia");
        ss >> inertia;
        if (inertia != Vec3r(0,0,0))
            rigid->inv_inertia = Vec3r(1.0/inertia.x, 1.0/inertia.y, 1.0/inertia.z);

        pElem2 = TiXmlHandle(pElem).FirstChild("node").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("node"))
        {
            point->inv_mass = rigid->inv_mass;
            ss.clear();
            ss << pElem2->Attribute("pos");
            ss >> node->pos;
            ++node;
            ++point;
            rigid->nodes_count++;
        }

        // Transform points positions
        phys->DoFrame0(*rigid);

        ++rigid;
    }

	// Init springs
	PhySpring *spring = phys->springs;
    mesh = hRoot->FirstChild("mesh").Element();
    for (; mesh; mesh = mesh->NextSiblingElement("mesh"))
    {
        pElem = TiXmlHandle(mesh).FirstChild("spring").Element();
        for (; pElem; pElem = pElem->NextSiblingElement("spring"))
        {
            stringstream ss;
            ss << pElem->Attribute("k");
            ss >> spring->k;
            spring->k *= phys->time * phys->time;
            ss.clear();
            ss << pElem->Attribute("d");
            ss >> spring->d;
            spring->d *= phys->time;
            ss.clear();
            ss << pElem->Attribute("s");
            ss >> spring->s;
            spring->s *= phys->time * phys->time;
            ss.clear();
            ss << pElem->Attribute("length");
            ss >> spring->l;

            spring->p1 = FindPoint(pElem->Attribute("p1"));
            spring->p2 = FindPoint(pElem->Attribute("p2"));

            ++spring;
        }
    }

	// Init joints
	PhyJoint *joint = phys->joints;
    mesh = hRoot->FirstChild("joints").Element();
    for (; mesh; mesh = mesh->NextSiblingElement("joints"))
    {
        pElem = TiXmlHandle(mesh).FirstChild("joint").Element();
        for (; pElem; pElem = pElem->NextSiblingElement("joint"))
        {
            stringstream ss;
            ss << pElem->Attribute("k");
            ss >> joint->k;
            joint->k *= phys->time * phys->time;
            ss.clear();
            ss << pElem->Attribute("d");
            ss >> joint->d;
            joint->d *= phys->time;
            ss.clear();
            ss << pElem->Attribute("s");
            ss >> joint->s;
            joint->s *= phys->time * phys->time;

            joint->p1 = FindPoint(pElem->Attribute("p1"));
            joint->p2 = FindPoint(pElem->Attribute("p2"));

            ++joint;
        }
    }

	// Init balloons
	PhyBalloon *balloon = phys->balloons;
	PhyPoint **ppoint = phys->ppoints;
    pElem = hRoot->FirstChild("balloon").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("balloon"))
    {
        balloon->points = ppoint;

        stringstream ss;
        ss << pElem->Attribute("pressure");
        ss >> balloon->pressure;

        // this is to avoid a multiplication each frame from the cross-product
        balloon->pressure *= 1.0/(2.0*3.0) * phys->time*phys->time;

        pElem2 = TiXmlHandle(pElem).FirstChild("point").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("point"))
        {
            *ppoint = FindPoint(pElem2->Attribute("p1"));
            ++ppoint;
            balloon->points_count++;
        }

        ++balloon;
    }

	// Init poses
    pElem = hRoot->FirstChild("pose").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("pose"))
    {
        stringstream ss;
        string name;
        ss << pElem->Attribute("name");
        ss >> name;

        if (name != "")
            poses[name].Load(TiXmlHandle(pElem), this);
    }

	// Init motors
    TypeName tn;
    tn.type = "rigid";
	PhyMotor *motor = phys->motors;
    pElem = hRoot->FirstChild("motor").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("motor"))
    {
        stringstream ss;
        ss << pElem->Attribute("rigid1");
        ss >> tn.name;
        if (tn.name != "")
            motor->r1 = &phys->rigids[namesIndex[tn]];
        ss.clear();
        ss << pElem->Attribute("rigid2");
        ss >> tn.name;
        if (tn.name != "")
            motor->r2 = &phys->rigids[namesIndex[tn]];

        ++motor;
    }

	// Init sounds
	PhySound *sound = phys->sounds;
    pElem = hRoot->FirstChild("sound").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("sound"))
    {
        sound->p1 = FindPoint(pElem->Attribute("p1"));
        sound->source = App::al.AddSound(pElem->Attribute("sound"), Vec3r(0, 0, 0));
        alSourcei(sound->source, AL_LOOPING, AL_TRUE);
        alSourcef(sound->source, AL_GAIN, 0);
        alSourcePlay(sound->source);

        ++sound;
    }

    // Init textures
    tn.type = "texture";
    pElem = hRoot->FirstChild("texture").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("texture"))
    {
        tn.name = pElem->Attribute("name");
        if ( tn.name != "" )
        {
            const char *tmp = pElem->Attribute("image");
            if (tmp != NULL)
            {
                textures[namesIndex[tn]].Aquire(tmp);
                continue;
            }
            /*
            tmp = pElem->Attribute("cubemap");
            if (tmp != NULL)
            {
                textures[inst.namesIndex[tn]].(tmp);
                continue;
            }
            */
            textures[namesIndex[tn]].Aquire(NULL);
        }
    }

    // Init triangles and quads
    {
        tn.type = "texture";
        RenderPass *renderpass = renderpasses;
        pElem = hRoot->FirstChild("render").Element();
        for (; pElem; pElem = pElem->NextSiblingElement("render"))
        {
            const char *tmp = pElem->Attribute("texture");
            if (tmp != NULL)
            {
                tn.name = tmp;
                if ( tn.name != "" )
                    renderpass->colormap = &textures[namesIndex[tn]];
            }

            tmp = pElem->Attribute("decalmap");
            if (tmp != NULL)
            {
                tn.name = tmp;
                if ( tn.name != "" )
                    renderpass->decalmap = &textures[namesIndex[tn]];
            }

            tmp = pElem->Attribute("envmap");
            if (tmp != NULL)
            {
                tn.name = tmp;
                if ( tn.name != "" )
                    renderpass->envmap = &textures[namesIndex[tn]];
            }

            renderpass++;
        }
    }

    {
        Vec3r **vert = gltriangle_verts;
        Vec3r **normal = gltriangle_normals;
        GLfloat *texcoord = gltexcoords;
        RenderPass *renderpass = renderpasses;
        pElem = hRoot->FirstChild("render").Element();
        for (; pElem; pElem = pElem->NextSiblingElement("render"))
        {
            pElem2 = TiXmlHandle(pElem).FirstChild("gltriangle").Element();
            for (; pElem2; pElem2 = pElem2->NextSiblingElement("gltriangle"))
            {
                vert[0] = &FindPoint(pElem2->Attribute("p1"))->pos;
                vert[1] = &FindPoint(pElem2->Attribute("p2"))->pos;
                vert[2] = &FindPoint(pElem2->Attribute("p3"))->pos;

                int n1 = 0, n2 = 0, n3 = 0;
                stringstream ss;
                if (pElem2->Attribute("normals"))
                {
                    ss << pElem2->Attribute("normals");
                    ss >> n1 >> n2 >> n3;
                    normal[0] = &glnormals[n1];
                    normal[1] = &glnormals[n2];
                    normal[2] = &glnormals[n3];
                }
                else
                {
                    normal[0] = glnormals + FindPointIndex(pElem2->Attribute("p1"));
                    normal[1] = glnormals + FindPointIndex(pElem2->Attribute("p2"));
                    normal[2] = glnormals + FindPointIndex(pElem2->Attribute("p3"));
                }

                ss.clear();
                ss << pElem2->Attribute("uvs");
                ss >> texcoord[0] >> texcoord[1];
                ss >> texcoord[2] >> texcoord[3];
                ss >> texcoord[4] >> texcoord[5];

                ss.clear();
                ss << pElem2->Attribute("uv1");
                ss >> texcoord[0] >> texcoord[1];
                ss.clear();
                ss << pElem2->Attribute("uv2");
                ss >> texcoord[2] >> texcoord[3];
                ss.clear();
                ss << pElem2->Attribute("uv3");
                ss >> texcoord[4] >> texcoord[5];

                vert += 3;
                normal += 3;
                texcoord += 6;
                renderpass->triangles_count++;
            }
            renderpass++;
        }

        vert = glquad_verts;
        normal = glquad_normals;
        renderpass = renderpasses;
        pElem = hRoot->FirstChild("render").Element();
        for (; pElem; pElem = pElem->NextSiblingElement("render"))
        {
            pElem2 = TiXmlHandle(pElem).FirstChild("glquad").Element();
            for (; pElem2; pElem2 = pElem2->NextSiblingElement("glquad"))
            {
                vert[0] = &FindPoint(pElem2->Attribute("p1"))->pos;
                vert[1] = &FindPoint(pElem2->Attribute("p2"))->pos;
                vert[2] = &FindPoint(pElem2->Attribute("p3"))->pos;
                vert[3] = &FindPoint(pElem2->Attribute("p4"))->pos;

                int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
                stringstream ss;
                if (pElem2->Attribute("normals"))
                {
                    ss << pElem2->Attribute("normals");
                    ss >> n1 >> n2 >> n3 >> n4;
                    normal[0] = &glnormals[n1];
                    normal[1] = &glnormals[n2];
                    normal[2] = &glnormals[n3];
                    normal[3] = &glnormals[n4];
                }
                else
                {
                    normal[0] = glnormals + FindPointIndex(pElem2->Attribute("p1"));
                    normal[1] = glnormals + FindPointIndex(pElem2->Attribute("p2"));
                    normal[2] = glnormals + FindPointIndex(pElem2->Attribute("p3"));
                    normal[3] = glnormals + FindPointIndex(pElem2->Attribute("p4"));
                }

                ss.clear();
                ss << pElem2->Attribute("uvs");
                ss >> texcoord[0] >> texcoord[1];
                ss >> texcoord[2] >> texcoord[3];
                ss >> texcoord[4] >> texcoord[5];
                ss >> texcoord[6] >> texcoord[7];

                ss.clear();
                ss << pElem2->Attribute("uv1");
                ss >> texcoord[0] >> texcoord[1];
                ss.clear();
                ss << pElem2->Attribute("uv2");
                ss >> texcoord[2] >> texcoord[3];
                ss.clear();
                ss << pElem2->Attribute("uv3");
                ss >> texcoord[4] >> texcoord[5];
                ss.clear();
                ss << pElem2->Attribute("uv4");
                ss >> texcoord[6] >> texcoord[7];

                vert += 4;
                normal += 4;
                texcoord += 8;
                renderpass->quads_count++;
            }
            renderpass++;
        }
    }

    // Init camera
    tn.type = "rigid";
    pElem = hRoot->FirstChild("camera").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("camera"))
    {
        const char *tmp = pElem->Attribute("name");
        if (tmp == NULL)
        {
            // Don't allow anonymous cameras
            continue;
        }

        // Find connected rigid
        tmp = pElem->Attribute("rigid");
        if (tmp == NULL)
            continue;

        tn.name = tmp;
        typeof(namesIndex.end()) it = namesIndex.find(tn);

        if (it == namesIndex.end())
            continue;

        // Create a camera
        Camera *cam = &cameras[tmp];
        cam->rigid = &phys->rigids[it->second];

        stringstream ss;
        if (pElem->Attribute("pos"))
        {
            ss << pElem->Attribute("pos");
            ss >> cam->pos;
            ss.clear();
        }

        if (pElem->Attribute("pos2"))
        {
            ss << pElem->Attribute("pos2");
            ss >> cam->pos2;
            ss.clear();
        }

        if (pElem->Attribute("orient"))
        {
            ss << pElem->Attribute("orient");
            ss >> cam->orient;
            ss.clear();
        }
    }

}

void PhyInstance::RecalculateNormals()
{
    for (Vec3r *it = glnormals, *end = glnormals+glnormals_count; it != end; ++it)
        it->SetToZero();

    Vec3r normal;

    typeof(gltriangle_verts) it = gltriangle_verts;
    typeof(gltriangle_verts) end = gltriangle_verts + gltriangle_verts_count;
    typeof(gltriangle_normals) it2 = gltriangle_normals;

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
        phys->insane = memPool_size;
        memcpy(memPool, memPool2, memPool_size);
        memcpy(&phys->profiler, profiler_buffer, sizeof(Profiler));
        //LOG("CrashHandling early end0 phys->insane: " << phys->insane);
        return;
    }

    // Save
    //LOG("CrashHandling save...");
    memcpy(memPool2, memPool, memPool_size);
    //LOG("CrashHandling end");
}
