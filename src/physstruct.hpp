#pragma once

#include <memory>
#include <mutex>

#include <GL/glew.h>
#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <math.h>
#include <tinyxml.h>

#include "physics.hpp"
#include "parser.hpp"
#include "pose.hpp"
#include "texture.hpp"

// lock mutex before access to phyInstances
extern std::mutex phyInstances_lock;

struct RenderPass
{
    unsigned int triangles_count;
    unsigned int quads_count;
    Texture *colormap;
    Texture *decalmap;
    Texture *envmap;
};

class Camera
{
public:
    Camera()
    {
        rigid = NULL;
        pos.SetToZero();
        pos2.SetToZero();
        orient.SetToZero();
    }
    PhyRigid *rigid;
    Vec3r pos;
    Vec3r pos2;
    Quat4r orient;
};

class PhyInstance
{
private:
    std::vector<char> memPool;
    std::vector<char> memPool2;

    PhyInstance(const PhyInstance& other) = delete; // Copy constructor
    PhyInstance& operator=(const PhyInstance& other) = delete; // Copy assignment operator

public:
    PhyInstance() = default;
    ~PhyInstance();

	std::map<TypeName, int> namesIndex;
	std::map<std::string, int> typeCount;
	std::map<std::string, Pose> poses;
    std::map<std::string, Camera> cameras;
    std::vector<Profiler> profilers;
    std::vector<Profiler>::iterator profiler_it;

	Physics *phys = nullptr;

    Vec3r **gltriangle_verts = nullptr;
    Vec3r **gltriangle_normals = nullptr;
    GLuint gltriangle_verts_count = 0;

    Vec3r **glquad_verts = nullptr;
    Vec3r **glquad_normals = nullptr;
    GLuint glquad_verts_count = 0;

    // normals can be shared between verts to allow smoothing
    Vec3r *glnormals = nullptr;
    GLuint glnormals_count = 0;

    // texcoords are not shared
    GLfloat *gltexcoords = nullptr;

    RenderPass *renderpasses = nullptr;
    unsigned int renderpasses_count = 0;

    Texture *textures = nullptr;
    GLuint textures_count = 0;

    void RecalculateNormals();
    std::vector<REAL> PollPhys(const char pollstring[]);
    void Remove();
    void CrashHandling();
    static void DeleteAll();

    static PhyInstance* InsertPhysXML(const char *filename);
    int UpdatePhys(const char name[]);
    int UpdatePhysBlend(const char name[], float a, float b);
private:
    PhyPoint* FindPoint(std::string name);
    int FindPointIndex(std::string name);
    static std::unique_ptr<PhyInstance> LoadPhysXML(const char *filename);
    void ParsePhysXML(TiXmlHandle *hRoot);
};

extern std::vector<std::unique_ptr<PhyInstance>> phyInstances;

bool InitPhys();
void QuitPhys();

void ParsePhysBlend(PhyInstance *inst, std::vector<std::string> *lines, float blend);
