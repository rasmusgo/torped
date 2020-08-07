#pragma once

#include "vectormath.hpp"
#include "profiler.hpp"

struct PhyPoint
{
    // position, dimension: L
    Vec3r pos;
    // velocity, dimension L NOT L/T
    Vec3r vel; // delta pos
    // acceleration, dimension L NOT L/T^2
    Vec3r acc;
    // force, dimension L*M NOT L*M/T^2
    Vec3r force; // delta delta pos * mass
    // mass, dimension M^-1
    REAL inv_mass; // 1 / mass
};

struct PhySpring
{
    // vilka punkter som är sammanlänkade
    PhyPoint *p1,*p2;
    // fjäderkonstant,  dämpningskonstant och stelhet (statisk kraft)
    REAL k, d, s;
    // avslappnad längd och aktuell längd
    REAL l, rl;
    //REAL maxforce, minforce;
    bool broken;
};

struct PhyJoint
{
    // vilka punkter som är sammanlänkade
    PhyPoint *p1,*p2;
    REAL k, d, s;
    bool broken;
};

struct PhyNode
{
    //PhyPoint point;
    Vec3r pos;
};

struct PhyRigid
{
    // vilka noder som är sammanlänkade
    PhyNode *nodes;
    // vilka punkter som är sammanlänkade (som noderna kontrollerar)
    PhyPoint *points;
    int nodes_count;
    REAL inv_mass;
    Vec3r inv_inertia;
    Vec3r pos;
    // velocity, dimension L NOT L/T
    Vec3r vel; // delta pos
    // acceleration, dimension L NOT L/T^2
    Vec3r acc;
    // force, dimension L NOT L/T^2
    Vec3r force; // delta delta pos
    Quat4r R_world_from_local;
    Vec3r angular_momentum;
    Vec3r spin; // angular velocity, accumulated torque / inertia, world space
    Vec3r torque; // world space
};

struct PhyBalloon
{
    // vilka punkter som är sammanlänkade
    PhyPoint **points;
    int points_count;
    REAL pressure; // actually, pressure / 2
};

struct PhyMotor
{
    // vilka rigids som är sammanlänkade
    PhyRigid *r1, *r2;
    // Vridmoment relativt r1 som ges på r2
    Vec3r torque;
    // Bromskraft
    Vec3r brake;
};

#define PHY_SOUND_SAMPLES 10
struct PhySound
{
    PhyPoint *p1;
    Vec3r buffer[PHY_SOUND_SAMPLES];
    int i;
    unsigned int source;
};

class Physics
{
public:
    PhyPoint *points;
    PhyNode *nodes;
    PhySpring *springs;
    PhyJoint *joints;
    PhyRigid *rigids;
    PhyBalloon *balloons;
    PhyPoint **ppoints;
    PhyMotor *motors;
    PhySound *sounds;

    unsigned int points_count;
    unsigned int nodes_count;
    unsigned int springs_count;
    unsigned int joints_count;
    unsigned int rigids_count;
    unsigned int balloons_count;
    unsigned int ppoints_count;
    unsigned int motors_count;
    unsigned int sounds_count;

    // gravity, dimension L NOT L/T^2
    Vec3r gravity;
    double timestep;
    double timestep_squared;
    REAL break_limit;
    REAL floor_k;
    REAL floor_d;
    REAL floor_friction;
    Vec3r bounds_max, bounds_min;

    int insane;

    Profiler profiler;

    Physics();
    void UpdateForces();
    void UpdateVelocitiesAndPositions();
    void TestBounds(Physics &a, const REAL r);
    void DoCollidePoints();
    void DoCollideTriangles();
    int CollideLineLine(PhyPoint &p1, PhyPoint &p2, PhyPoint &p3, PhyPoint &p4);
    void Move(const Vec3r &offset);
    void Rotate(const Quat4r &offset);
    void CollideFloor();

    void UpdatePointsFromRigid(PhyRigid &rigid);
private:
    inline void UpdateVelocity(PhyPoint &point);
    inline void UpdatePosition(PhyPoint &point);
    inline void UpdateForces(PhySpring &spring);
    inline void UpdateForces(PhyJoint &joint);
    inline void UpdateForces(PhyBalloon &balloon);
    inline void UpdateForces(PhyMotor &motor);
    inline void UpdateSound(PhySound &sound);

    inline void UpdateVelocity(PhyRigid &rigid);
    inline void UpdatePositionAndOrientation(PhyRigid &rigid);

    inline void UpdateBounds(const Vec3r &a);
    inline void UpdateBounds(const PhyPoint &point);
};
