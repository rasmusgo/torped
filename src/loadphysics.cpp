#include "loadphysics.hpp"

#include <memory>

#include "alstruct.hpp"
#include "json.hpp"
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

namespace configuru {
    template<>
    inline Vec3r as(const Config& config)
    {
        auto&& array = config.as_array();
        config.check(array.size() == 3, "Expected Vec3r");
        return Vec3r((REAL)array[0], (REAL)array[1], (REAL)array[2]);
    }

    template<>
    inline Quat4r as(const Config& config)
    {
        auto&& array = config.as_array();
        config.check(array.size() == 4, "Expected Quat4r");
        return Quat4r((REAL)array[0], (REAL)array[1], (REAL)array[2], (REAL)array[3]);
    }
}

template<size_t N>
inline std::array<std::string, N> StringArray(const Json& json)
{
    auto&& array = json.as_array();
    json.check(array.size() == N, "Unexpected size of array");
    std::array<std::string, N> ret;
    for (int i = 0; i < N; ++i)
    {
        ret[i] = array[i].as_string();
    }
    return ret;
}

template <class T>
inline T* AddAndIncrement(char * &place, const int count)
{
    if (count == 0)
        return NULL;

    T *ret = new (place) T[count];
    place += sizeof(T) * count;
    return ret;
}

void AllocateMemory(PhyInstance *inst);

std::unique_ptr<PhyInstance> LoadPhysXML(const char *filename)
{
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
        pElem = TiXmlHandle(mesh).FirstChild(typeName.type).Element();
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
                inst->namesIndex[typeName] = inst->typeCount[typeName.type];
            inst->typeCount[typeName.type]++;
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

    AllocateMemory(inst.get());
    ParsePhysXML(inst.get(), &hRoot);

    //PrintPhys(&inst, &ofstream("loadphysxml.txt"));

    VLOG_S(3) << "End of LoadPhysXML";
    return inst; // success
}

void AllocateMemory(PhyInstance *inst)
{
    CHECK_NOTNULL_F(inst);
    // calculate required storage size
    unsigned int size =
        sizeof(Physics) +
        sizeof(PhyPoint)    * inst->typeCount["point"]      +
        sizeof(PhyPoint)    * inst->typeCount["node"]       +
        sizeof(PhyNode)     * inst->typeCount["node"]       +
        sizeof(PhySpring)   * inst->typeCount["spring"]     +
        sizeof(PhyJoint)    * inst->typeCount["joint"]      +
        sizeof(PhyRigid)    * inst->typeCount["rigid"]      +
        sizeof(PhyBalloon)  * inst->typeCount["balloon"]    +
        sizeof(PhyPoint*)   * inst->typeCount["ppoint"]     +
        sizeof(PhyMotor)    * inst->typeCount["motor"]      +
        sizeof(PhySound)    * inst->typeCount["sound"]      +
        // vertex and normal pointers
        sizeof(Vec3r*) * 6  * inst->typeCount["gltriangle"] +
        sizeof(Vec3r*) * 8  * inst->typeCount["glquad"]     +
        // gltexcoords
        sizeof(GLfloat) * 6 * inst->typeCount["gltriangle"] +
        sizeof(GLfloat) * 8 * inst->typeCount["glquad"]     +
        // glnormals
        sizeof(Vec3r)       * inst->typeCount["glnormal"]   +
        sizeof(Texture)     * inst->typeCount["texture"]    +
        sizeof(RenderPass)  * inst->typeCount["render"]     +
        0;

    // allocate memory for all the physics stuff
    // double the size to keep a copy of memPool to allow crash replay
    inst->memPool.assign(size, 0);
    inst->memPool2.assign(size, 0);

    // insertion place in memory
    char * place = inst->memPool.data();

    // create the physics instance
    inst->phys = AddAndIncrement<Physics>(place, 1);

    // create points and stuff
    // add points controlled by nodes after regular points
    inst->phys->points           = AddAndIncrement<PhyPoint   >(place, inst->typeCount["point"]+inst->typeCount["node"]);
    inst->phys->nodes            = AddAndIncrement<PhyNode    >(place, inst->typeCount["node"]);
    inst->phys->springs          = AddAndIncrement<PhySpring  >(place, inst->typeCount["spring"]);
    inst->phys->joints           = AddAndIncrement<PhyJoint   >(place, inst->typeCount["joint"]);
    inst->phys->rigids           = AddAndIncrement<PhyRigid   >(place, inst->typeCount["rigid"]);
    inst->phys->balloons         = AddAndIncrement<PhyBalloon >(place, inst->typeCount["balloon"]);
    inst->phys->ppoints          = AddAndIncrement<PhyPoint*  >(place, inst->typeCount["ppoint"]);
    inst->phys->motors           = AddAndIncrement<PhyMotor   >(place, inst->typeCount["motor"]);
    inst->phys->sounds           = AddAndIncrement<PhySound   >(place, inst->typeCount["sound"]);
    inst->gltriangle_verts       = AddAndIncrement<Vec3r*     >(place, inst->typeCount["gltriangle"]*3);
    inst->gltriangle_normals     = AddAndIncrement<Vec3r*     >(place, inst->typeCount["gltriangle"]*3);
    inst->glquad_verts           = AddAndIncrement<Vec3r*     >(place, inst->typeCount["glquad"]*4);
    inst->glquad_normals         = AddAndIncrement<Vec3r*     >(place, inst->typeCount["glquad"]*4);
    inst->gltexcoords            = AddAndIncrement<GLfloat    >(place, inst->typeCount["gltriangle"]*6+inst->typeCount["glquad"]*8);
    inst->glnormals              = AddAndIncrement<Vec3r      >(place, inst->typeCount["glnormal"]);
    inst->textures               = AddAndIncrement<Texture    >(place, inst->typeCount["texture"]);
    inst->renderpasses           = AddAndIncrement<RenderPass >(place, inst->typeCount["render"]);

    // remember count
    inst->phys->points_count     = inst->typeCount["point"];
    inst->phys->nodes_count      = inst->typeCount["node"];
    inst->phys->springs_count    = inst->typeCount["spring"];
    inst->phys->joints_count     = inst->typeCount["joint"];
    inst->phys->rigids_count     = inst->typeCount["rigid"];
    inst->phys->balloons_count   = inst->typeCount["balloon"];
    inst->phys->ppoints_count    = inst->typeCount["ppoint"];
    inst->phys->motors_count     = inst->typeCount["motor"];
    inst->phys->sounds_count     = inst->typeCount["sound"];
    inst->gltriangle_verts_count = inst->typeCount["gltriangle"]*3;
    inst->glquad_verts_count     = inst->typeCount["glquad"]*4;
    inst->glnormals_count        = inst->typeCount["glnormal"];
    inst->textures_count         = inst->typeCount["texture"];
    inst->renderpasses_count     = inst->typeCount["render"];
}

void ParsePhysXML(PhyInstance *inst, TiXmlHandle *hRoot)
{
    CHECK_NOTNULL_F(inst);
    CHECK_NOTNULL_F(inst->phys);
    Physics *phys = inst->phys;
    if (auto gravity = hRoot->Element()->Attribute("gravity"))
    {
        std::stringstream ss;
        Vec3r tmp = phys->gravity / phys->time * phys->time;
        ss << gravity;
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
        if (auto orient = pElem->Attribute("rot"))
        {
            rigid->orient = Quat4r(1,0,0,0);
            ss << orient;
            ss >> rigid->orient;
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
        rigid->angular_momentum = Mat3x3r(rigid->orient).sandwich(rigid->inv_inertia) * rigid->spin; // inv_inertia is not inverted yet.
        if (rigid->inv_inertia != Vec3r(0,0,0))
            rigid->inv_inertia = Vec3r(1.0/rigid->inv_inertia.x, 1.0/rigid->inv_inertia.y, 1.0/rigid->inv_inertia.z);

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
            std::stringstream ss;
            if (auto k = pElem->Attribute("k"))
            {
                ss << k;
                ss >> spring->k;
                spring->k *= phys->time * phys->time;
                ss.clear();
            }
            if (auto d = pElem->Attribute("d"))
            {
                ss << d;
                ss >> spring->d;
                spring->d *= phys->time;
                ss.clear();
            }
            if (auto s = pElem->Attribute("s"))
            {
                ss << s;
                ss >> spring->s;
                spring->s *= phys->time * phys->time;
                ss.clear();
            }
            if (auto length = pElem->Attribute("length"))
            {
                ss << length;
                ss >> spring->l;
            }

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
                joint->k *= phys->time * phys->time;
                ss.clear();
            }
            if (auto d = pElem->Attribute("d"))
            {
                ss << d;
                ss >> joint->d;
                joint->d *= phys->time;
                ss.clear();
            }
            if (auto s = pElem->Attribute("s"))
            {
                ss << s;
                ss >> joint->s;
                joint->s *= phys->time * phys->time;
            }

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
        balloon->pressure *= 1.0/(2.0*3.0) * phys->time*phys->time;

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
                inst->textures[inst->namesIndex[tn]].Aquire(tmp);
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
            inst->textures[inst->namesIndex[tn]].Aquire(NULL);
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

void RegisterNamedElements(PhyInstance &inst, const Json &json, const std::string &array_name, const std::string &type)
{
    if (json.has_key(array_name))
    {
        TypeName tn;
        tn.type = type;
        for (auto json_element : json[array_name].as_array())
        {
            tn.name = json_element["name"].as_string();
            inst.namesIndex[tn] = inst.typeCount[tn.type];
            inst.typeCount[tn.type]++;
        }
    }
}

std::unique_ptr<PhyInstance> LoadPhysJSON(const char *filename)
{
    ERROR_CONTEXT("LoadPhysJSON", filename);
    std::unique_ptr<PhyInstance> inst(new PhyInstance());

    char* buffer = NULL;
    PhysFSLoadFile(filename, buffer);
    CHECK_NOTNULL_F(buffer, "Failed to open '%s'", filename);
    std::unique_ptr<char> file_buffer_pointer{buffer};

    Json json = configuru::parse_string(buffer, configuru::JSON, filename);
    CHECK_F(json.is_object());

    if (json.has_key("meshes"))
    {
        for (auto json_mesh : json["meshes"].as_array())
        {
            // Count points and springs
            RegisterNamedElements(*inst, json_mesh, "points", "point");
            RegisterNamedElements(*inst, json_mesh, "springs", "spring");
        }
    }

    // Count rigids and nodes
    RegisterNamedElements(*inst, json, "rigids", "rigid");
    if (json.has_key("rigids"))
    {
        for (auto json_rigid : json["rigids"].as_array())
        {
            RegisterNamedElements(*inst, json_rigid, "nodes", "node");
        }
    }

    // Count joints
    RegisterNamedElements(*inst, json, "joints", "joint");

    // Count balloons and their triangles (points)
    RegisterNamedElements(*inst, json, "balloons", "balloon");
    if (json.has_key("balloons"))
    {
        for (auto json_balloon : json["balloons"].as_array())
        {
            inst->typeCount["ppoint"] += json_balloon["points"].as_array().size();
        }
    }

    // Count motors
    RegisterNamedElements(*inst, json, "motors", "motor");

    // Count sounds
    RegisterNamedElements(*inst, json, "sounds", "sound");

    // Count textures
    RegisterNamedElements(*inst, json, "textures", "texture");

    // Count renderpasses, triangles, quads and normals
    if (json.has_key("renderpasses"))
    {
        TypeName tn;
        tn.type = "render";
        inst->typeCount[tn.type] = json["renderpasses"].as_array().size();

        for (auto json_renderpass : json["renderpasses"].as_array())
        {
            RegisterNamedElements(*inst, json_renderpass, "gltriangles", "gltriangle");
            if (json_renderpass.has_key("gltriangles"))
            {
                for (auto json_gltriangle : json_renderpass["gltriangles"].as_array())
                {
                    if (json_gltriangle.has_key("normals"))
                    {
                        std::array<int, 3> normal_indices = json_gltriangle["normals"];
                        for (int index : normal_indices)
                        {
                            if (inst->typeCount["glnormal"] <= index)
                            {
                                inst->typeCount["glnormal"] = index + 1;
                            }
                        }
                    }
                    else
                    {
                        std::array<std::string, 3> point_names = StringArray<3>(json_gltriangle["points"]);
                        for (int i = 0; i < 3; ++i)
                        {
                            int index = inst->FindPointIndex(point_names[i]);
                            if (inst->typeCount["glnormal"] <= index)
                            {
                                inst->typeCount["glnormal"] = index + 1;
                            }
                        }
                    }
                }
            }
            RegisterNamedElements(*inst, json_renderpass, "glquads", "glquad");
            if (json_renderpass.has_key("glquads"))
            {
                for (auto json_glquad : json_renderpass["glquads"].as_array())
                {
                    if (json_glquad.has_key("normals"))
                    {
                        std::array<int, 4> normal_indices = json_glquad["normals"];
                        for (int index : normal_indices)
                        {
                            if (inst->typeCount["glnormal"] <= index)
                            {
                                inst->typeCount["glnormal"] = index + 1;
                            }
                        }
                    }
                    else
                    {
                        std::array<std::string, 4> point_names = StringArray<4>(json_glquad["points"]);
                        for (int i = 0; i < 4; ++i)
                        {
                            int index = inst->FindPointIndex(point_names[i]);
                            if (inst->typeCount["glnormal"] <= index)
                            {
                                inst->typeCount["glnormal"] = index + 1;
                            }
                        }
                    }
                }
            }
        }
    }

    AllocateMemory(inst.get());
    ParsePhysJSON(inst.get(), json);

    return inst; // success
}

void ParsePhysJSON(PhyInstance *inst, const Json &json)
{
    CHECK_NOTNULL_F(inst);
    CHECK_NOTNULL_F(inst->phys);
    Physics *phys = inst->phys;
    if (json.has_key("gravity"))
    {
        phys->gravity = Vec3r(json["gravity"]) * phys->time * phys->time;
    }

    // Init points
    PhyPoint *point_it = phys->points;
    if (json.has_key("meshes"))
    {
        for (auto json_mesh : json["meshes"].as_array())
        {
            for (auto json_point : json_mesh["points"].as_array())
            {
                point_it->pos = Vec3r(json_point["pos"]);
                // TODO: premultiply with timestep
                if (json_point.has_key("vel"))
                {
                    point_it->vel = Vec3r(json_point["vel"]);
                }
                // TODO: premultiply with timestep
                if (json_point.has_key("force"))
                {
                    point_it->force = Vec3r(json_point["force"]);
                }
                if (json_point.has_key("mass"))
                {
                    point_it->inv_mass = json_point["mass"];
                    if (point_it->inv_mass != 0)
                    {
                        point_it->inv_mass = REAL{1} / point_it->inv_mass;
                    }
                }
                ++point_it;
            }
        }
    }

    // Init rigids and nodes
    if (json.has_key("rigids"))
    {
        PhyRigid *rigid_it = phys->rigids;
        PhyNode *node_it = phys->nodes;
        for (auto json_rigid : json["rigids"].as_array())
        {
            rigid_it->nodes = node_it;
            rigid_it->points = point_it;
            rigid_it->pos = Vec3r(json_rigid["pos"]);

            // TODO: premultiply with timestep
            if (json_rigid.has_key("vel"))
            {
                rigid_it->vel = Vec3r(json_rigid["vel"]);
            }
            // TODO: premultiply with timestep
            if (json_rigid.has_key("force"))
            {
                rigid_it->force = Vec3r(json_rigid["force"]);
            }
            if (json_rigid.has_key("rot"))
            {
                rigid_it->orient = Quat4r(json_rigid["rot"]);
            }
            // TODO: premultiply with timestep
            if (json_rigid.has_key("spin"))
            {
                rigid_it->spin = Vec3r(json_rigid["spin"]);
            }
            // TODO: premultiply with timestep
            if (json_rigid.has_key("torque"))
            {
                rigid_it->torque = Vec3r(json_rigid["torque"]);
            }
            if (json_rigid.has_key("mass"))
            {
                rigid_it->inv_mass = json_rigid["mass"];
                if (rigid_it->inv_mass != 0)
                {
                    rigid_it->inv_mass = REAL{1} / rigid_it->inv_mass;
                }
            }

            if (json_rigid.has_key("inertia"))
            {
                rigid_it->inv_inertia = Vec3r(json_rigid["inertia"]);  // Inverted below.
            }

            for (auto json_node : json_rigid["nodes"].as_array())
            {
                point_it->inv_mass = rigid_it->inv_mass;
                node_it->pos = Vec3r(json_node["pos"]);
                ++node_it;
                ++point_it;
                rigid_it->nodes_count++;
            }

            // Compute angular momentum
            rigid_it->angular_momentum = Mat3x3r(rigid_it->orient).sandwich(rigid_it->inv_inertia) * rigid_it->spin; // inv_inertia is not inverted yet.
            if (rigid_it->inv_inertia != Vec3r(0,0,0))
                rigid_it->inv_inertia = Vec3r(1.0/rigid_it->inv_inertia.x, 1.0/rigid_it->inv_inertia.y, 1.0/rigid_it->inv_inertia.z);

            // Transform points positions
            phys->DoFrame0(*rigid_it);
            ++rigid_it;
        }
        json["rigids"].check_dangling();
    }

    // Init springs
    if (json.has_key("meshes"))
    {
        PhySpring *spring_it = phys->springs;
        for (auto json_mesh : json["meshes"].as_array())
        {
            if (json_mesh.has_key("springs"))
            {
                for (auto json_spring : json_mesh["springs"].as_array())
                {
                    spring_it->k = REAL(json_spring["k"]) * phys->time * phys->time;
                    spring_it->d = REAL(json_spring["d"]) * phys->time;
                    spring_it->s = REAL(json_spring["s"]) * phys->time * phys->time;
                    spring_it->l = json_spring["length"];
                    spring_it->p1 = inst->FindPoint(json_spring["p1"].as_string());
                    spring_it->p2 = inst->FindPoint(json_spring["p2"].as_string());
                    ++spring_it;
                }
            }
        }
        json["meshes"].check_dangling();
    }

    // Init joints
    if (json.has_key("joints"))
    {
        PhyJoint *joint_it = phys->joints;
        for (auto json_joint : json["joints"].as_array())
        {
            joint_it->k = REAL(json_joint["k"]) * phys->time * phys->time;
            joint_it->d = REAL(json_joint["d"]) * phys->time;
            joint_it->s = REAL(json_joint["s"]) * phys->time * phys->time;
            joint_it->p1 = inst->FindPoint(json_joint["p1"]);
            joint_it->p2 = inst->FindPoint(json_joint["p2"]);
            ++joint_it;
        }
        json["joints"].check_dangling();
    }

    // Init balloons
    if (json.has_key("balloons"))
    {
        PhyBalloon *balloon_it = phys->balloons;
        PhyPoint **ppoint_it = phys->ppoints;
        for (auto json_balloon : json["balloons"].as_array())
        {
            balloon_it->points = ppoint_it;
            balloon_it->pressure = json_balloon["pressure"];

            // this is to avoid a multiplication each frame from the cross-product
            balloon_it->pressure *= 1.0/(2.0*3.0) * phys->time*phys->time;

            for (auto json_point : json_balloon["points"].as_array())
            {
                *ppoint_it = inst->FindPoint(json_point);
                ++ppoint_it;
                balloon_it->points_count++;
            }
            ++balloon_it;
        }
        json["balloons"].check_dangling();
    }

    // Init poses
    if (json.has_key("poses"))
    {
        for (auto json_pose : json["poses"].as_array())
        {
            if (json_pose.has_key("name"))
            {
                //inst->poses[json_pose["name"]] = PoseFromJSON(json_pose);
            }
        }
        //json["poses"].check_dangling();
    }

    // Init motors
    if (json.has_key("motors"))
    {
        TypeName tn;
        tn.type = "rigid";
        PhyMotor *motor_it = phys->motors;
        for (auto json_motors : json["motors"].as_array())
        {
            tn.name = json_motors["rigid1"].as_string();
            motor_it->r1 = &phys->rigids[inst->namesIndex[tn]];
            tn.name = json_motors["rigid2"].as_string();
            motor_it->r2 = &phys->rigids[inst->namesIndex[tn]];
            ++motor_it;
        }
        json["motors"].check_dangling();
    }

    // Init sounds
    CHECK_NOTNULL_F(alstruct_ptr);
    if (json.has_key("sounds"))
    {
        PhySound *sound_it = phys->sounds;
        for (auto json_sound : json["sounds"].as_array())
        {
            sound_it->p1 = inst->FindPoint(json_sound["point"]);
            sound_it->source = alstruct_ptr->AddSound(json_sound["filename"].c_str(), Vec3r(0, 0, 0));
            alSourcei(sound_it->source, AL_LOOPING, AL_TRUE);
            alSourcef(sound_it->source, AL_GAIN, 0);
            alSourcePlay(sound_it->source);
            ++sound_it;
        }
        json["sounds"].check_dangling();
    }

    // Init textures
    if (json.has_key("textures"))
    {
        TypeName tn;
        tn.type = "texture";
        for (auto json_texture : json["textures"].as_array())
        {
            tn.name = json_texture["name"].as_string();
            inst->textures[inst->namesIndex[tn]].Aquire(json_texture["filename"].c_str());
        }
        json["textures"].check_dangling();
    }

    // Init triangles and quads
    if (json.has_key("renderpasses"))
    {
        TypeName tn;
        tn.type = "texture";
        RenderPass *renderpass_it = inst->renderpasses;
        Vec3r **triangle_vert_it = inst->gltriangle_verts;
        Vec3r **triangle_normal_it = inst->gltriangle_normals;
        GLfloat *triangle_texcoord_it = inst->gltexcoords;
        Vec3r **quad_vert_it = inst->glquad_verts;
        Vec3r **quad_normal_it = inst->glquad_normals;
        GLfloat *quad_texcoord_it = inst->gltexcoords + inst->gltriangle_verts_count * 2;

        for (auto json_renderpass : json["renderpasses"].as_array())
        {
            if (json_renderpass.has_key("texture"))
            {
                tn.name = json_renderpass["texture"].as_string();
                renderpass_it->colormap = &inst->textures[inst->namesIndex[tn]];
            }
            if (json_renderpass.has_key("decalmap"))
            {
                tn.name = json_renderpass["decalmap"].as_string();
                renderpass_it->decalmap = &inst->textures[inst->namesIndex[tn]];
            }
            if (json_renderpass.has_key("envmap"))
            {
                tn.name = json_renderpass["envmap"].as_string();
                renderpass_it->envmap = &inst->textures[inst->namesIndex[tn]];
            }

            if (json_renderpass.has_key("gltriangles"))
            {
                for (auto json_gltriangle : json_renderpass["gltriangles"].as_array())
                {
                    std::array<std::string, 3> point_names = StringArray<3>(json_gltriangle["points"]);
                    for (int i = 0; i < 3; ++i)
                    {
                        triangle_vert_it[i] = &inst->FindPoint(point_names[i])->pos;
                    }

                    if (json_gltriangle.has_key("normals"))
                    {
                        std::array<int, 3> normal_indices = json_gltriangle["normals"];
                        for (int i = 0; i < 3; ++i)
                        {
                            triangle_normal_it[i] = &inst->glnormals[normal_indices[i]];
                        }
                    }
                    else
                    {
                        for (int i = 0; i < 3; ++i)
                        {
                            triangle_normal_it[i] = inst->glnormals + inst->FindPointIndex(point_names[i]);
                        }
                    }

                    std::array<REAL, 6> uvs = json_gltriangle["uvs"];
                    for (int i = 0; i < 6; ++i)
                    {
                        triangle_texcoord_it[i] = uvs[i];
                    }

                    triangle_vert_it += 3;
                    triangle_normal_it += 3;
                    triangle_texcoord_it += 6;
                    renderpass_it->triangles_count++;
                }
            }
            if (json_renderpass.has_key("glquads"))
            {
                for (auto json_glquad : json_renderpass["glquads"].as_array())
                {
                    std::array<std::string, 4> point_names = StringArray<4>(json_glquad["points"]);
                    for (int i = 0; i < 4; ++i)
                    {
                        quad_vert_it[i] = &inst->FindPoint(point_names[i])->pos;
                    }

                    if (json_glquad.has_key("normals"))
                    {
                        std::array<int, 4> normal_indices = json_glquad["normals"];
                        for (int i = 0; i < 4; ++i)
                        {
                            quad_normal_it[i] = &inst->glnormals[normal_indices[i]];
                        }
                    }
                    else
                    {
                        for (int i = 0; i < 4; ++i)
                        {
                            quad_normal_it[i] = inst->glnormals + inst->FindPointIndex(point_names[i]);
                        }
                    }

                    std::array<REAL, 8> uvs = json_glquad["uvs"];
                    for (int i = 0; i < 8; ++i)
                    {
                        quad_texcoord_it[i] = uvs[i];
                    }

                    quad_vert_it += 4;
                    quad_normal_it += 4;
                    quad_texcoord_it += 8;
                    renderpass_it->quads_count++;
                }
            }
            renderpass_it++;
        }
        json["renderpasses"].check_dangling();
    }

    // Init camera
    if (json.has_key("cameras"))
    {
        TypeName tn;
        tn.type = "rigid";
        for (auto json_camera : json["cameras"].as_array())
        {
            // Create a camera
            Camera *cam = &inst->cameras[json_camera["name"].as_string()];

            // Find connected rigid
            tn.name = json_camera["rigid"].as_string();
            cam->rigid = &phys->rigids[inst->namesIndex[tn]];
            if (json_camera.has_key("pos"))
            {
                cam->pos = Vec3r(json_camera["pos"]);
            }
            if (json_camera.has_key("pos2"))
            {
                cam->pos2 = Vec3r(json_camera["pos2"]);
            }
            if (json_camera.has_key("orient"))
            {
                cam->orient = Quat4r(json_camera["orient"]);
            }
        }
        json["cameras"].check_dangling();
    }
}
