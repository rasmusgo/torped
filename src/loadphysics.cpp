#include "loadphysics.hpp"

#include "pch.hpp"

#include "alstruct.hpp"
#include "logging.hpp"
#include "physfsstruct.hpp"
#include "physics.hpp"
#include "physstruct.hpp"

namespace {
    AlStruct* alstruct_ptr = nullptr;
}

void PhyInstance::SetAlStructPtr(AlStruct* alstruct)
{
    alstruct_ptr = alstruct;
}

template <class T>
inline T* CreateAndIncrement(char * &place, const int count)
{
    if (count == 0)
        return NULL;

    // Skip ahead to next aligned position
    static_assert(sizeof(size_t) == sizeof(char*), "");
    const size_t remainder = reinterpret_cast<size_t>(place) % alignof(T);
    if (remainder != 0)
        place += alignof(T) - remainder;

    CHECK_EQ_F(reinterpret_cast<size_t>(place) % alignof(T), 0);

    // Create objects
    // Run placement new on each object separately to avoid issues related to
    // __STDCPP_DEFAULT_NEW_ALIGNMENT__ which a bitch in visual studio.
    // T *ret = new (place) T[count]; // <- This causes problems!
    T *ret = reinterpret_cast<T*>(place);
    CHECK_EQ_F(reinterpret_cast<size_t>(place), reinterpret_cast<size_t>(ret));
    for (int i = 0; i < count; ++i)
    {
        CHECK_F(new (&ret[i]) T == &ret[i]);
    }

    // Account for all elements
    const size_t stride =
        (sizeof(T) + alignof(T) - 1) / alignof(T) * alignof(T);
    CHECK_GE_F(stride, sizeof(T));
    place += stride * (count - 1) + sizeof(T);

    return ret;
}

template <class T>
inline void IncrementAlignedSpace(size_t &offset, const int count)
{
    if (count == 0)
        return;

    // Skip ahead to next aligned position
    const size_t remainder = offset % alignof(T);
    if (remainder != 0)
        offset += alignof(T) - remainder;

    // Account for all elements
    const size_t stride =
        (sizeof(T) + alignof(T) - 1) / alignof(T) * alignof(T);
    CHECK_GE_F(stride, sizeof(T));
    offset += stride * (count - 1) + sizeof(T);
}

std::unique_ptr<PhyInstance> LoadPhysXML(const char *filename)
{
    //LOG(__FUNCTION__ << ": " << __LINE__);

    std::unique_ptr<PhyInstance> inst(new PhyInstance());

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
        pElem = TiXmlHandle(mesh).FirstChild(typeName.type.c_str()).Element();
        for (; pElem; pElem = pElem->NextSiblingElement("point"))
        {
            typeName.name = pElem->Attribute("name");
            if ( typeName.name != "" )
                inst->namesIndex[typeName] = inst->typeCount[typeName.type];
            inst->typeCount[typeName.type]++;
        }
    }

	// Count rigids and nodes
    typeName.type = "rigid";
    pElem = hRoot.FirstChild("rigid").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("rigid"))
    {
        typeName.name = pElem->Attribute("name");
        if ( typeName.name != "" )
            inst->namesIndex[typeName] = inst->typeCount[typeName.type];
        inst->typeCount[typeName.type]++;

        TypeName tn;
        tn.type = "node";
        pElem2 = TiXmlHandle(pElem).FirstChild("node").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("node"))
        {
            tn.name = pElem2->Attribute("name");
            if ( tn.name != "" )
                inst->namesIndex[tn] = inst->typeCount[tn.type];
            inst->typeCount[tn.type]++;
        }
    }

    // Count stiffs and nodes
    typeName.type = "stiff";
    pElem = hRoot.FirstChild("stiff").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("stiff"))
    {
        typeName.name = pElem->Attribute("name");
        if ( typeName.name != "" )
            inst->namesIndex[typeName] = inst->typeCount[typeName.type];
        inst->typeCount[typeName.type]++;

        TypeName tn;
        tn.type = "node";
        pElem2 = TiXmlHandle(pElem).FirstChild("node").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("node"))
        {
            tn.name = pElem2->Attribute("name");
            if ( tn.name != "" )
                inst->namesIndex[tn] = inst->typeCount[tn.type];
            inst->typeCount[tn.type]++;
        }
    }

	// Count springs
    typeName.type = "spring";
    mesh = hRoot.FirstChild("mesh").Element();
    for (; mesh; mesh = mesh->NextSiblingElement("mesh"))
    {
        pElem = TiXmlHandle(mesh).FirstChild(typeName.type.c_str()).Element();
        for (; pElem; pElem = pElem->NextSiblingElement("spring"))
        {
            typeName.name = pElem->Attribute("name");
            if ( typeName.name != "" )
                inst->namesIndex[typeName] = inst->typeCount[typeName.type];
            inst->typeCount[typeName.type]++;
        }
    }

	// Count joints
    typeName.type = "joint";
    mesh = hRoot.FirstChild("joints").Element();
    for (; mesh; mesh = mesh->NextSiblingElement("joints"))
    {
        pElem = TiXmlHandle(mesh).FirstChild(typeName.type.c_str()).Element();
        for (; pElem; pElem = pElem->NextSiblingElement("joint"))
        {
            typeName.name = pElem->Attribute("name");
            if ( typeName.name != "" )
                inst->namesIndex[typeName] = inst->typeCount["joint"];
            inst->typeCount["joint"]++;
        }
    }

	// Count balloons and their triangles (points)
    typeName.type = "balloon";
    pElem = hRoot.FirstChild("balloon").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("balloon"))
    {
        typeName.name = pElem->Attribute("name");
        if ( typeName.name != "" )
            inst->namesIndex[typeName] = inst->typeCount[typeName.type];
        inst->typeCount[typeName.type]++;

        TypeName tn;
        tn.type = "ppoint";
        pElem2 = TiXmlHandle(pElem).FirstChild("point").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("point"))
        {
            inst->typeCount[tn.type]++;
        }
    }

	// Count motors
    typeName.type = "motor";
    pElem = hRoot.FirstChild("motor").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("motor"))
    {
        typeName.name = pElem->Attribute("name");
        if ( typeName.name != "" )
            inst->namesIndex[typeName] = inst->typeCount[typeName.type];
        inst->typeCount[typeName.type]++;
    }

	// Count sounds
    typeName.type = "sound";
    pElem = hRoot.FirstChild("sound").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("sound"))
    {
        typeName.name = pElem->Attribute("name");
        if ( typeName.name != "" )
            inst->namesIndex[typeName] = inst->typeCount[typeName.type];
        inst->typeCount[typeName.type]++;
    }

    // Count textures
    typeName.type = "texture";
    pElem = hRoot.FirstChild("texture").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("texture"))
    {
        typeName.name = pElem->Attribute("name");
        if ( typeName.name != "" )
        {
            inst->namesIndex[typeName] = inst->typeCount["texture"];
            inst->typeCount["texture"]++;
        }
    }

    // Count renderpasses, triangles, quads and normals
    pElem = hRoot.FirstChild("render").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("render"))
    {
        inst->typeCount["render"]++;

        pElem2 = TiXmlHandle(pElem).FirstChild("gltriangle").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("gltriangle"))
        {
            inst->typeCount["gltriangle"]++;
            int n1 = 0, n2 = 0, n3 = 0;

            if (pElem2->Attribute("normals") != NULL)
            {
                std::stringstream ss;
                ss << pElem2->Attribute("normals");
                ss >> n1 >> n2 >> n3;
            }
            else
            {
                n1 = inst->FindPointIndex(pElem2->Attribute("p1"));
                n2 = inst->FindPointIndex(pElem2->Attribute("p2"));
                n3 = inst->FindPointIndex(pElem2->Attribute("p3"));
            }

            if (inst->typeCount["glnormal"] <= n1)
                inst->typeCount["glnormal"] = n1+1;
            if (inst->typeCount["glnormal"] <= n2)
                inst->typeCount["glnormal"] = n2+1;
            if (inst->typeCount["glnormal"] <= n3)
                inst->typeCount["glnormal"] = n3+1;
        }

        pElem2 = TiXmlHandle(pElem).FirstChild("glquad").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("glquad"))
        {
            inst->typeCount["glquad"]++;

            int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
            if (pElem2->Attribute("normals") != NULL)
            {
                std::stringstream ss;
                ss << pElem2->Attribute("normals");
                ss >> n1 >> n2 >> n3 >> n4;
            }
            else
            {
                n1 = inst->FindPointIndex(pElem2->Attribute("p1"));
                n2 = inst->FindPointIndex(pElem2->Attribute("p2"));
                n3 = inst->FindPointIndex(pElem2->Attribute("p3"));
                n4 = inst->FindPointIndex(pElem2->Attribute("p4"));
            }

            if (inst->typeCount["glnormal"] <= n1)
                inst->typeCount["glnormal"] = n1+1;
            if (inst->typeCount["glnormal"] <= n2)
                inst->typeCount["glnormal"] = n2+1;
            if (inst->typeCount["glnormal"] <= n3)
                inst->typeCount["glnormal"] = n3+1;
            if (inst->typeCount["glnormal"] <= n4)
                inst->typeCount["glnormal"] = n4+1;
        }
    }

    // calculate required storage size
    size_t size = alignof(Physics) - 1;
    IncrementAlignedSpace<Physics    >(size, 1);
    IncrementAlignedSpace<PhyPoint   >(size, inst->typeCount["point"]+inst->typeCount["node"]);
    IncrementAlignedSpace<PhyNode    >(size, inst->typeCount["node"]);
    IncrementAlignedSpace<PhySpring  >(size, inst->typeCount["spring"]);
    IncrementAlignedSpace<PhyJoint   >(size, inst->typeCount["joint"]);
    IncrementAlignedSpace<PhyRigid   >(size, inst->typeCount["rigid"]);
    IncrementAlignedSpace<PhyRigid   >(size, inst->typeCount["stiff"]);
    IncrementAlignedSpace<PhyBalloon >(size, inst->typeCount["balloon"]);
    IncrementAlignedSpace<PhyPoint*  >(size, inst->typeCount["ppoint"]);
    IncrementAlignedSpace<PhyMotor   >(size, inst->typeCount["motor"]);
    IncrementAlignedSpace<PhySound   >(size, inst->typeCount["sound"]);
    IncrementAlignedSpace<Vec3r*     >(size, inst->typeCount["gltriangle"]*3);
    IncrementAlignedSpace<Vec3r*     >(size, inst->typeCount["gltriangle"]*3);
    IncrementAlignedSpace<Vec3r*     >(size, inst->typeCount["glquad"]*4);
    IncrementAlignedSpace<Vec3r*     >(size, inst->typeCount["glquad"]*4);
    IncrementAlignedSpace<GLfloat    >(size, inst->typeCount["gltriangle"]*6+inst->typeCount["glquad"]*8);
    IncrementAlignedSpace<Vec3r      >(size, inst->typeCount["glnormal"]);
    IncrementAlignedSpace<Texture    >(size, inst->typeCount["texture"]);
    IncrementAlignedSpace<RenderPass >(size, inst->typeCount["render"]);

    // allocate memory for all the physics stuff
    // double the size to keep a copy of memPool to allow crash replay
    inst->memPool.assign(size, 0);
    inst->memPool2.assign(size, 0);

    // insertion place in memory
    char * place = inst->memPool.data();

    // create the physics instance
    inst->phys = CreateAndIncrement<Physics>(place, 1);

    // create points and stuff
    // add points controlled by nodes after regular points
    inst->phys->points           = CreateAndIncrement<PhyPoint   >(place, inst->typeCount["point"]+inst->typeCount["node"]);
    inst->phys->nodes            = CreateAndIncrement<PhyNode    >(place, inst->typeCount["node"]);
    inst->phys->springs          = CreateAndIncrement<PhySpring  >(place, inst->typeCount["spring"]);
    inst->phys->joints           = CreateAndIncrement<PhyJoint   >(place, inst->typeCount["joint"]);
    inst->phys->rigids           = CreateAndIncrement<PhyRigid   >(place, inst->typeCount["rigid"]);
    inst->phys->stiffs           = CreateAndIncrement<PhyStiff   >(place, inst->typeCount["stiff"]);
    inst->phys->balloons         = CreateAndIncrement<PhyBalloon >(place, inst->typeCount["balloon"]);
    inst->phys->ppoints          = CreateAndIncrement<PhyPoint*  >(place, inst->typeCount["ppoint"]);
    inst->phys->motors           = CreateAndIncrement<PhyMotor   >(place, inst->typeCount["motor"]);
    inst->phys->sounds           = CreateAndIncrement<PhySound   >(place, inst->typeCount["sound"]);
    inst->gltriangle_verts       = CreateAndIncrement<Vec3r*     >(place, inst->typeCount["gltriangle"]*3);
    inst->gltriangle_normals     = CreateAndIncrement<Vec3r*     >(place, inst->typeCount["gltriangle"]*3);
    inst->glquad_verts           = CreateAndIncrement<Vec3r*     >(place, inst->typeCount["glquad"]*4);
    inst->glquad_normals         = CreateAndIncrement<Vec3r*     >(place, inst->typeCount["glquad"]*4);
    inst->gltexcoords            = CreateAndIncrement<GLfloat    >(place, inst->typeCount["gltriangle"]*6+inst->typeCount["glquad"]*8);
    inst->glnormals              = CreateAndIncrement<Vec3r      >(place, inst->typeCount["glnormal"]);
    inst->textures               = CreateAndIncrement<Texture    >(place, inst->typeCount["texture"]);
    inst->renderpasses           = CreateAndIncrement<RenderPass >(place, inst->typeCount["render"]);

    CHECK_LE_F(place - inst->memPool.data(), static_cast<ptrdiff_t>(size));
    CHECK_LE_F(place, inst->memPool.data() + size);

    // remember count
    inst->phys->points_count     = inst->typeCount["point"];
    inst->phys->nodes_count      = inst->typeCount["node"];
    inst->phys->springs_count    = inst->typeCount["spring"];
    inst->phys->joints_count     = inst->typeCount["joint"];
    inst->phys->rigids_count     = inst->typeCount["rigid"];
    inst->phys->stiffs_count     = inst->typeCount["stiff"];
    inst->phys->balloons_count   = inst->typeCount["balloon"];
    inst->phys->ppoints_count    = inst->typeCount["ppoint"];
    inst->phys->motors_count     = inst->typeCount["motor"];
    inst->phys->sounds_count     = inst->typeCount["sound"];
    inst->gltriangle_verts_count = inst->typeCount["gltriangle"]*3;
    inst->glquad_verts_count     = inst->typeCount["glquad"]*4;
    inst->glnormals_count        = inst->typeCount["glnormal"];
    inst->textures_count         = inst->typeCount["texture"];
    inst->renderpasses_count     = inst->typeCount["render"];

    ParsePhysXML(inst.get(), &hRoot);

    //PrintPhys(&inst, &ofstream("loadphysxml.txt"));

    // Store data for crash handling
    CHECK_F(!inst->phys->insane);
    inst->CrashHandling();

    VLOG_S(3) << "End of LoadPhysXML";
    return inst; // success
}

void ParsePhysXML(PhyInstance *inst, TiXmlHandle *hRoot)
{
    CHECK_NOTNULL_F(inst);
    CHECK_NOTNULL_F(inst->phys);
    Physics *phys = inst->phys;
    if (auto gravity = hRoot->Element()->Attribute("gravity"))
    {
        std::stringstream ss;
        Vec3r tmp = phys->gravity / phys->timestep_squared;
        ss << gravity;
        ss >> tmp;
        phys->gravity = tmp * phys->timestep_squared;
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
            std::stringstream ss;
            if (auto pos = pElem->Attribute("pos"))
            {
                ss << pos;
                ss >> point->pos;
                ss.clear();
            }
            else
            {
                ABORT_S() << "Point has no position (\"pos\")";
            }
            // TODO: premultiply with timestep
            if (auto vel = pElem->Attribute("vel"))
            {
                ss << vel;
                ss >> point->vel;
                ss.clear();
            }
            // TODO: premultiply with timestep
            if (auto force = pElem->Attribute("force"))
            {
                ss << force;
                ss >> point->force;
                ss.clear();
            }
            if (auto mass = pElem->Attribute("mass"))
            {
                ss << mass;
                ss >> point->inv_mass;
                if (point->inv_mass != 0)
                {
                    point->inv_mass = REAL{1} / point->inv_mass;
                }
            }
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
        std::stringstream ss;

        if (auto pos = pElem->Attribute("pos"))
        {
            ss << pos;
            ss >> rigid->pos;
            ss.clear();
        }
        // TODO: premultiply with timestep
        if (auto vel = pElem->Attribute("vel"))
        {
            ss << vel;
            ss >> rigid->vel;
            ss.clear();
        }
        // TODO: premultiply with timestep
        if (auto force = pElem->Attribute("force"))
        {
            ss << force;
            ss >> rigid->force;
            ss.clear();
        }
        if (auto rot = pElem->Attribute("rot"))
        {
            rigid->R_world_from_local = Quat4r(1,0,0,0);
            ss << rot;
            ss >> rigid->R_world_from_local;
            ss.clear();
        }
        // TODO: premultiply with timestep
        if (auto spin = pElem->Attribute("spin"))
        {
            ss << spin;
            ss >> rigid->spin;
            ss.clear();
        }
        // TODO: premultiply with timestep
        if (auto torque = pElem->Attribute("torque"))
        {
            ss << torque;
            ss >> rigid->torque;
            ss.clear();
        }
        if (auto mass = pElem->Attribute("mass"))
        {
            ss << mass;
            ss >> rigid->inv_mass;
            if (rigid->inv_mass != 0)
                rigid->inv_mass = REAL{1} / rigid->inv_mass;
            ss.clear();
        }

        if (auto inertia = pElem->Attribute("inertia"))
        {
            ss << inertia;
            ss >> rigid->inv_inertia; // Inverted below.
        }

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

        // Compute angular momentum
        rigid->angular_momentum = Mat3x3r(rigid->R_world_from_local).sandwich(rigid->inv_inertia) * rigid->spin; // inv_inertia is not inverted yet.
        if (rigid->inv_inertia != Vec3r(0,0,0))
            rigid->inv_inertia = Vec3r(1.0/rigid->inv_inertia.x, 1.0/rigid->inv_inertia.y, 1.0/rigid->inv_inertia.z);

        // Transform points positions
        phys->UpdatePointsFromRigid(*rigid);

        ++rigid;
    }

    // Init stiffs and nodes
    PhyStiff *stiff = phys->stiffs;
    pElem = hRoot->FirstChild("stiff").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("stiff"))
    {
        stiff->nodes = node;
        stiff->points = point;
        std::stringstream ss;

        if (auto pos = pElem->Attribute("pos"))
        {
            ss << pos;
            ss >> stiff->pos;
            ss.clear();
        }
        if (auto orient = pElem->Attribute("rot"))
        {
            stiff->orient = Quat4r(1,0,0,0);
            ss << orient;
            ss >> stiff->orient;
            ss.clear();
        }
        if (auto mass = pElem->Attribute("mass"))
        {
            ss << mass;
            ss >> stiff->inv_mass;
            ss.clear();
            if (stiff->inv_mass != 0)
                stiff->inv_mass = REAL{1} / stiff->inv_mass;
        }
        if (auto alpha = pElem->Attribute("alpha"))
        {
            ss << alpha;
            ss >> stiff->alpha;
            ss.clear();
        }
        if (auto beta = pElem->Attribute("beta"))
        {
            ss << beta;
            ss >> stiff->beta;
            ss.clear();
        }
        if (auto damping = pElem->Attribute("damping"))
        {
            ss << damping;
            ss >> stiff->damping;
            ss.clear();
        }

        pElem2 = TiXmlHandle(pElem).FirstChild("node").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("node"))
        {
            ss.clear();
            ss << pElem2->Attribute("pos");
            ss >> node->pos;
            ++node;
            ++point;
            stiff->nodes_count++;
        }

        // Transform points positions
        phys->InitializeStiff(*stiff);

        ++stiff;
    }

	// Init springs
	PhySpring *spring = phys->springs;
    mesh = hRoot->FirstChild("mesh").Element();
    for (; mesh; mesh = mesh->NextSiblingElement("mesh"))
    {
        pElem = TiXmlHandle(mesh).FirstChild("spring").Element();
        for (; pElem; pElem = pElem->NextSiblingElement("spring"))
        {
            std::stringstream ss;
            if (auto k = pElem->Attribute("k"))
            {
                ss << k;
                ss >> spring->k;
                spring->k *= phys->timestep_squared;
                ss.clear();
            }
            if (auto d = pElem->Attribute("d"))
            {
                ss << d;
                ss >> spring->d;
                spring->d *= phys->timestep;
                ss.clear();
            }
            if (auto s = pElem->Attribute("s"))
            {
                ss << s;
                ss >> spring->s;
                spring->s *= phys->timestep_squared;
                ss.clear();
            }
            if (auto length = pElem->Attribute("length"))
            {
                ss << length;
                ss >> spring->l;
            }

            // HACK to compensate for simplified physics timestep.
            spring->k *= 2;
            spring->d *= 2;
            spring->s *= 2;

            spring->p1 = inst->FindPoint(pElem->Attribute("p1"));
            spring->p2 = inst->FindPoint(pElem->Attribute("p2"));

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
            std::stringstream ss;
            if (auto k = pElem->Attribute("k"))
            {
                ss << k;
                ss >> joint->k;
                joint->k *= phys->timestep_squared;
                ss.clear();
            }
            if (auto d = pElem->Attribute("d"))
            {
                ss << d;
                ss >> joint->d;
                joint->d *= phys->timestep;
                ss.clear();
            }
            if (auto s = pElem->Attribute("s"))
            {
                ss << s;
                ss >> joint->s;
                joint->s *= phys->timestep_squared;
            }

            // HACK to compensate for simplified physics timestep.
            joint->k *= 3;
            joint->d *= 3;
            joint->s *= 3;

            joint->p1 = inst->FindPoint(pElem->Attribute("p1"));
            joint->p2 = inst->FindPoint(pElem->Attribute("p2"));

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

        std::stringstream ss;
        ss << pElem->Attribute("pressure");
        ss >> balloon->pressure;

        // this is to avoid a multiplication each frame from the cross-product
        balloon->pressure *= 1.0/(2.0*3.0) * phys->timestep_squared;

        pElem2 = TiXmlHandle(pElem).FirstChild("point").Element();
        for (; pElem2; pElem2 = pElem2->NextSiblingElement("point"))
        {
            *ppoint = inst->FindPoint(pElem2->Attribute("p1"));
            ++ppoint;
            balloon->points_count++;
        }

        ++balloon;
    }

	// Init poses
    pElem = hRoot->FirstChild("pose").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("pose"))
    {
        std::stringstream ss;
        std::string name;
        ss << pElem->Attribute("name");
        ss >> name;

        if (name != "")
            inst->poses[name].Load(TiXmlHandle(pElem), inst);
    }

	// Init motors
    TypeName tn;
    tn.type = "rigid";
	PhyMotor *motor = phys->motors;
    pElem = hRoot->FirstChild("motor").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("motor"))
    {
        std::stringstream ss;
        ss << pElem->Attribute("rigid1");
        ss >> tn.name;
        ss.clear();
        if (tn.name != "")
            motor->r1 = &phys->rigids[inst->namesIndex[tn]];
        ss << pElem->Attribute("rigid2");
        ss >> tn.name;
        ss.clear();
        if (tn.name != "")
            motor->r2 = &phys->rigids[inst->namesIndex[tn]];

        ++motor;
    }

	// Init sounds
    CHECK_NOTNULL_F(alstruct_ptr);
	PhySound *sound = phys->sounds;
    pElem = hRoot->FirstChild("sound").Element();
    for (; pElem; pElem = pElem->NextSiblingElement("sound"))
    {
        sound->p1 = inst->FindPoint(pElem->Attribute("p1"));
        sound->source = alstruct_ptr->AddSound(pElem->Attribute("sound"), Vec3r(0, 0, 0));
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
                inst->textures[inst->namesIndex[tn]].Acquire(tmp);
                continue;
            }
            /*
            tmp = pElem->Attribute("cubemap");
            if (tmp != NULL)
            {
                textures[inst->namesIndex[tn]].(tmp);
                continue;
            }
            */
            inst->textures[inst->namesIndex[tn]].Acquire(NULL);
        }
    }

    // Init triangles and quads
    {
        tn.type = "texture";
        RenderPass *renderpass = inst->renderpasses;
        pElem = hRoot->FirstChild("render").Element();
        for (; pElem; pElem = pElem->NextSiblingElement("render"))
        {
            const char *tmp = pElem->Attribute("texture");
            if (tmp != NULL)
            {
                tn.name = tmp;
                if ( tn.name != "" )
                    renderpass->colormap = &inst->textures[inst->namesIndex[tn]];
            }

            tmp = pElem->Attribute("decalmap");
            if (tmp != NULL)
            {
                tn.name = tmp;
                if ( tn.name != "" )
                    renderpass->decalmap = &inst->textures[inst->namesIndex[tn]];
            }

            tmp = pElem->Attribute("envmap");
            if (tmp != NULL)
            {
                tn.name = tmp;
                if ( tn.name != "" )
                    renderpass->envmap = &inst->textures[inst->namesIndex[tn]];
            }

            renderpass++;
        }
    }

    {
        Vec3r **vert = inst->gltriangle_verts;
        Vec3r **normal = inst->gltriangle_normals;
        GLfloat *texcoord = inst->gltexcoords;
        RenderPass *renderpass = inst->renderpasses;
        pElem = hRoot->FirstChild("render").Element();
        for (; pElem; pElem = pElem->NextSiblingElement("render"))
        {
            pElem2 = TiXmlHandle(pElem).FirstChild("gltriangle").Element();
            for (; pElem2; pElem2 = pElem2->NextSiblingElement("gltriangle"))
            {
                vert[0] = &inst->FindPoint(pElem2->Attribute("p1"))->pos;
                vert[1] = &inst->FindPoint(pElem2->Attribute("p2"))->pos;
                vert[2] = &inst->FindPoint(pElem2->Attribute("p3"))->pos;

                int n1 = 0, n2 = 0, n3 = 0;
                std::stringstream ss;
                if (auto normals = pElem2->Attribute("normals"))
                {
                    ss << normals;
                    ss >> n1 >> n2 >> n3;
                    ss.clear();
                    normal[0] = &inst->glnormals[n1];
                    normal[1] = &inst->glnormals[n2];
                    normal[2] = &inst->glnormals[n3];
                }
                else
                {
                    normal[0] = inst->glnormals + inst->FindPointIndex(pElem2->Attribute("p1"));
                    normal[1] = inst->glnormals + inst->FindPointIndex(pElem2->Attribute("p2"));
                    normal[2] = inst->glnormals + inst->FindPointIndex(pElem2->Attribute("p3"));
                }

                if (auto uvs = pElem2->Attribute("uvs"))
                {
                    ss << uvs;
                    ss >> texcoord[0] >> texcoord[1];
                    ss >> texcoord[2] >> texcoord[3];
                    ss >> texcoord[4] >> texcoord[5];
                    ss.clear();
                }

                if (auto uv1 = pElem2->Attribute("uv1"))
                {
                    ss << uv1;
                    ss >> texcoord[0] >> texcoord[1];
                    ss.clear();
                }
                if (auto uv2 = pElem2->Attribute("uv2"))
                {
                    ss << uv2;
                    ss >> texcoord[2] >> texcoord[3];
                    ss.clear();
                }
                if (auto uv3 = pElem2->Attribute("uv3"))
                {
                    ss << uv3;
                    ss >> texcoord[4] >> texcoord[5];
                    ss.clear();
                }

                vert += 3;
                normal += 3;
                texcoord += 6;
                renderpass->triangles_count++;
            }
            renderpass++;
        }

        vert = inst->glquad_verts;
        normal = inst->glquad_normals;
        renderpass = inst->renderpasses;
        pElem = hRoot->FirstChild("render").Element();
        for (; pElem; pElem = pElem->NextSiblingElement("render"))
        {
            pElem2 = TiXmlHandle(pElem).FirstChild("glquad").Element();
            for (; pElem2; pElem2 = pElem2->NextSiblingElement("glquad"))
            {
                vert[0] = &inst->FindPoint(pElem2->Attribute("p1"))->pos;
                vert[1] = &inst->FindPoint(pElem2->Attribute("p2"))->pos;
                vert[2] = &inst->FindPoint(pElem2->Attribute("p3"))->pos;
                vert[3] = &inst->FindPoint(pElem2->Attribute("p4"))->pos;

                int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
                std::stringstream ss;
                if (pElem2->Attribute("normals"))
                {
                    ss << pElem2->Attribute("normals");
                    ss >> n1 >> n2 >> n3 >> n4;
                    ss.clear();

                    normal[0] = &inst->glnormals[n1];
                    normal[1] = &inst->glnormals[n2];
                    normal[2] = &inst->glnormals[n3];
                    normal[3] = &inst->glnormals[n4];
                }
                else
                {
                    normal[0] = inst->glnormals + inst->FindPointIndex(pElem2->Attribute("p1"));
                    normal[1] = inst->glnormals + inst->FindPointIndex(pElem2->Attribute("p2"));
                    normal[2] = inst->glnormals + inst->FindPointIndex(pElem2->Attribute("p3"));
                    normal[3] = inst->glnormals + inst->FindPointIndex(pElem2->Attribute("p4"));
                }

                if (auto uvs = pElem2->Attribute("uvs"))
                {
                    ss << uvs;
                    ss >> texcoord[0] >> texcoord[1];
                    ss >> texcoord[2] >> texcoord[3];
                    ss >> texcoord[4] >> texcoord[5];
                    ss >> texcoord[6] >> texcoord[7];
                    ss.clear();
                }

                if (auto uv1 = pElem2->Attribute("uv1"))
                {
                    ss << uv1;
                    ss >> texcoord[0] >> texcoord[1];
                    ss.clear();
                }
                if (auto uv2 = pElem2->Attribute("uv2"))
                {
                    ss << uv2;
                    ss >> texcoord[2] >> texcoord[3];
                    ss.clear();
                }
                if (auto uv3 = pElem2->Attribute("uv3"))
                {
                    ss << uv3;
                    ss >> texcoord[4] >> texcoord[5];
                    ss.clear();
                }
                if (auto uv4 = pElem2->Attribute("uv4"))
                {
                    ss << uv4;
                    ss >> texcoord[6] >> texcoord[7];
                    ss.clear();
                }

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
        auto it = inst->namesIndex.find(tn);

        if (it == inst->namesIndex.end())
            continue;

        // Create a camera
        Camera *cam = &inst->cameras[tmp];
        cam->rigid = &phys->rigids[it->second];

        std::stringstream ss;
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
