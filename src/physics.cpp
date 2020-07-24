#include "pch.hpp"

#include "physics.hpp"
//#include "logging.hpp"


Physics::Physics(): gravity(0,0,0), time(0.001), break_limit(0.05), insane(0)
{
/*
    REAL infinity = std::numeric_limits<REAL>::infinity();
    bounds_max = Vec3r(-infinity, -infinity, -infinity);
    bounds_min = Vec3r(infinity, infinity, infinity);
/*/
    bounds_max = Vec3r(0, 0, 0);
    bounds_min = Vec3r(0, 0, 0);
//*/
}

    //App::console << "entering DoFrame\n";

//#define DEBUGPRINT(a) (App::console << a);
#define DEBUGPRINT(a) ;
/*
#define DO(func,param,from,to) \
    { \
        DEBUGPRINT("before " << #func << "(" << #param << ")" << std::endl) \
        for(unsigned int i = (from); i < (to); i++) \
            (func)( (param)[i] ); \
        if (App::developermode) \
        { \
            profiler.RememberTime(#func" "#param); \
        } \
    }
/*/
#define DO(func,param,from,to) \
    { \
        DEBUGPRINT("before " << #func << "(" << #param << ")" << std::endl) \
        decltype(param) it = (param)+(from); \
        decltype(param) end = (param)+(to); \
        while (it != end) \
        { \
            (func)( *it ); \
            ++it; \
        } \
        profiler.RememberTime(#func" "#param); \
    }
//        for(unsigned int i = (from); i < (to); i++)
//*/

inline bool IsSane(const Vec3r &a)
{
    // check for infinity and NaN
    if ( a.x < a.x+1 &&
         a.y < a.y+1 &&
         a.z < a.z+1 )
         return true;
    return false;
}

void Physics::DoFrame1()
{
    profiler.Reset();

    if (insane)
    {
        std::stringstream ss;
        ss << "Insane: " << insane;
        //std::cerr << ss.str() << std::endl;
        profiler.RememberTime(ss.str());
        return;
    }

    if (!IsSane(bounds_max) || !IsSane(bounds_min) )
    {
        insane = 1;
        return;
    }

    DO(DoFrame1, sounds, 0, sounds_count)

    // uppdatera noderna
    //DO(DoFrame0, rigids, 0, rigids_count)

    // räkna ut linjernas krafter på deras punkter
    //assert(springs != 0 || springs_count == 0);
    DO(DoFrame1, springs, 0, springs_count)

    // Räkna ut ballongernas kraft på deras punkter
    DO(DoFrame1, balloons, 0, balloons_count)

    // Låt motorerna arbeta
    DO(DoFrame1, motors, 0, motors_count)

    // håll ihop saker
    DO(DoFrame1, joints, 0, joints_count)
    DO(DoFrame1, joints, 0, joints_count)

    // räkna ut acceleration och hastighet (och gravitation)
    DO(DoFrame1, points, 0, points_count)
    DO(DoFrame1, rigids, 0, rigids_count)

    // håll ihop saker
    DO(DoFrame2, springs, 0, springs_count)
    DO(DoFrame2, joints, 0, joints_count)

    // räkna ut acceleration och hastighet (ej gravitation)
    DO(DoFrame2, points, 0, points_count)
    DO(DoFrame2, rigids, 0, rigids_count)
}

void Physics::DoFrame2()
{
    profiler.SkipTime();

    if (insane)
        return;

    // räkna ut acceleration och hastighet (ej gravitation)
    DO(DoFrame2, points, 0, points_count)
    DO(DoFrame2, rigids, 0, rigids_count)

    // räkna ut position
    DO(DoFrame3, points, 0, points_count)
    DO(DoFrame3, rigids, 0, rigids_count)

    // uppdatera noderna
    DO(DoFrame0, rigids, 0, rigids_count)

    REAL infinity = std::numeric_limits<REAL>::infinity();
    bounds_max = Vec3r(-infinity, -infinity, -infinity);
    bounds_min = Vec3r(infinity, infinity, infinity);

    DO(UpdateBounds, points, 0, points_count+nodes_count)
}

#undef REMEMBER_TIME
#undef DO

inline void Physics::DoFrame1(PhyPoint &point)
{
    // uppdatera hastighet
    if (point.inv_mass == 0)
    {
        point.force.SetToZero();
        return;
    }

    if (!IsSane(point.force))
    {
        insane = 2;
        point.force.SetToZero();
    }

    point.acc = point.force * point.inv_mass + gravity;
    point.vel += point.acc;

    // nollställ krafter
    point.force.SetToZero();
}

// same as above except gravity and damping
inline void Physics::DoFrame2(PhyPoint &point)
{
    // uppdatera hastighet
    if (point.inv_mass == 0)
    {
        point.force.SetToZero();
        return;
    }

    if (!IsSane(point.force))
    {
        insane = 3;
        point.force.SetToZero();
    }

    point.acc = point.force * point.inv_mass;
    point.vel += point.acc;

    // nollställ krafter
    point.force.SetToZero();
}

inline void Physics::DoFrame3(PhyPoint &point)
{
    // uppdatera position
    //point.pos += time * point.vel;
    point.pos += point.vel;
    //point.pos2 = point.pos + point.vel;
}

template <class T>
inline void UpdateMax(T &a, const T &b)
{
    if (b > a)
        a = b;
}

template <class T>
inline void UpdateMin(T &a, const T &b)
{
    if (b < a)
        a = b;
}

inline void Physics::UpdateBounds(const Vec3r &a)
{
    UpdateMax(bounds_max.x, a.x);
    UpdateMax(bounds_max.y, a.y);
    UpdateMax(bounds_max.z, a.z);

    UpdateMin(bounds_min.x, a.x);
    UpdateMin(bounds_min.y, a.y);
    UpdateMin(bounds_min.z, a.z);
}

inline void Physics::UpdateBounds(const PhyPoint &point)
{
    UpdateBounds(point.pos + point.vel);
}

inline void Physics::DoFrame1(PhySpring &spring)
{
    if (spring.broken)
        return;

    //Vec3r velAB(spring.p2->vel - spring.p1->vel);
    // NOTE: vel har inte dimensionen L/T utan bara L så time ska inte vara med
    Vec3r AB2(spring.p2->pos - spring.p1->pos);// + velAB);

    // avståndet mellan punkterna
    spring.rl = AB2.FastLength();

    REAL deviation = spring.rl - spring.l;

    // hur stor kraft som ska korrigera felet
    REAL force = deviation*spring.k;

    if (deviation > 0)
    {
        force += spring.s;
    }
    else if (deviation < 0)
    {
        force -= spring.s;
    }

    // lägg till kraften efter normaliserad riktning
    // undvik division med 0
    if ( 0 == spring.rl )
    {
        //App::console << "premature return DoFrame1(spring)" << std::endl;
        return;
    }

    //AB2 /= spring.rl;
    //Vec3r forceVec = AB2 * ( force + AB2*velAB * spring.d ));
    //Vec3r forceVec = AB2 * ( (force + AB2*velAB * spring.d / spring.rl) / spring.rl );

    Vec3r forceVec = AB2 * ( force / spring.rl );

    spring.p1->force += forceVec;
    spring.p2->force -= forceVec;
}

inline void Physics::DoFrame2(PhySpring &spring)
{
    if (spring.broken)
        return;

    Vec3r velAB(spring.p2->vel - spring.p1->vel);
    // NOTE: vel har inte dimensionen L/T utan bara L så time ska inte vara med
    Vec3r AB2(spring.p2->pos - spring.p1->pos);// + velAB);

    // lägg till kraften efter normaliserad riktning
    // undvik division med 0
    Vec3r forceVec;
    if ( 0 == spring.rl )
        forceVec = velAB * spring.d;
    else
        forceVec = AB2 * ( (AB2*velAB) * spring.d / (spring.rl*spring.rl) );

    spring.p1->force += forceVec;
    spring.p2->force -= forceVec;

    /*
    if (spring.broken)
        return;
    Vec3r fAB(spring.p2->force - spring.p1->force);
    Vec3r forceVec = fAB * 0.5;
    spring.p1->force += forceVec;
    spring.p2->force -= forceVec;
    */
}

inline void Physics::DoFrame1(PhyJoint &joint)
{
    if (joint.broken)
        return;

    Vec3r AB(joint.p2->pos - joint.p1->pos);// + joint.p2->vel - joint.p1->vel);

    REAL length = AB.FastLength();
    /*
    if (length > break_limit) // BREAK
    {
        joint.broken = true;
        return;
    }
    */
    Vec3r forceVec(AB * joint.k);
    if (length > 0.0001)
        forceVec += AB * (joint.s / length);

    joint.p1->force += forceVec;
    joint.p2->force -= forceVec;
}

inline void Physics::DoFrame2(PhyJoint &joint)
{
    if (joint.broken)
        return;
    Vec3r velAB(joint.p2->vel - joint.p1->vel);
    Vec3r forceVec = velAB * joint.d;
    joint.p1->force += forceVec;
    joint.p2->force -= forceVec;
}

inline void Physics::DoFrame3(PhyJoint &joint)
{
    if (joint.broken)
        return;
    Vec3r fAB(joint.p2->force - joint.p1->force);
    Vec3r forceVec = fAB * 0.5;
    joint.p1->force += forceVec;
    joint.p2->force -= forceVec;
}

inline void Physics::DoFrame1(PhyBalloon &balloon)
{
    PhyPoint **points = balloon.points;
    PhyPoint **end = points + balloon.points_count;

/* 0.037
    std::stringstream ss;
    ss << "pressure: " << balloon.pressure;
    profiler.RememberTime(ss.str().c_str());
*/

    while (points < end)
    {
        Vec3r AB = points[1]->pos - points[0]->pos;
        Vec3r BC = points[2]->pos - points[1]->pos;

        Vec3r force = (AB).Cross(BC) * balloon.pressure;

        points[0]->force += force;
        points[1]->force += force;
        points[2]->force += force;

        points += 3;
    }
}

inline void Physics::DoFrame1(PhyMotor &motor)
{
    Mat3x3r orientationMatrix(motor.r1->orient);
    Vec3r torque = orientationMatrix * motor.torque;
    motor.r1->torque -= torque;
    motor.r2->torque += torque;
}

inline void Physics::DoFrame1(PhySound &sound)
{
    sound.buffer[sound.i] = sound.p1->vel;
    sound.i = (sound.i+1) % PHY_SOUND_SAMPLES;
}

void Physics::DoFrame0(PhyRigid &rigid)
{
    Mat3x3r orientationMatrix(rigid.orient);
    PhyNode *node = rigid.nodes;
    PhyPoint *point = rigid.points;
    PhyPoint *end = point + rigid.nodes_count;

    while (point != end)
    {
        point->pos = orientationMatrix * node->pos; // glöm inte  att addera pos
        point->vel = rigid.vel + (rigid.spin).Cross(point->pos);
        point->pos += rigid.pos; // glöm inte  att addera pos
        point++;
        node++;
    }
}

inline void Physics::DoFrame1(PhyRigid &rigid)
{
    PhyPoint *point = rigid.points;
    PhyPoint *end = point + rigid.nodes_count;
    while (point != end)
    {
        if (!IsSane(point->force))
        {
            insane = 4;
            point->force.SetToZero();
        }

        rigid.force += point->force;
        rigid.torque += (point->pos-rigid.pos).Cross(point->force);
        point->force.SetToZero();
        point++;
    }

    //rigid.vel += (rigid.force/rigid.mass + gravity) * time;
    rigid.vel += rigid.force * rigid.inv_mass + gravity;

    //rigid.angular_momentum += rigid.torque * time;
    rigid.angular_momentum += rigid.torque;
    rigid.spin = Mat3x3r(rigid.orient).sandwich(rigid.inv_inertia) * rigid.angular_momentum;

    rigid.force.SetToZero();
    rigid.torque.SetToZero();
}

// same as above except this has no gravity
inline void Physics::DoFrame2(PhyRigid &rigid)
{
    PhyPoint *point = rigid.points;
    PhyPoint *end = point + rigid.nodes_count;
    while (point != end)
    {
        if (!IsSane(point->force))
        {
            insane = 5;
            point->force.SetToZero();
        }

        rigid.force += point->force;
        rigid.torque += (point->pos-rigid.pos).Cross(point->force);
        point->force.SetToZero();
        point++;
    }

    rigid.vel += rigid.force * rigid.inv_mass;

    //rigid.angular_momentum += rigid.torque * time;
    rigid.angular_momentum += rigid.torque;
    rigid.spin = Mat3x3r(rigid.orient).sandwich(rigid.inv_inertia) * rigid.angular_momentum;

    rigid.force.SetToZero();
    rigid.torque.SetToZero();
}

inline void Physics::DoFrame3(PhyRigid &rigid)
{
    if (!IsSane(rigid.vel))
    {
        insane = 6;
        return;
    }

    if (!IsSane(rigid.pos))
    {
        insane = 7;
        return;
    }

    if (!IsSane(rigid.spin))
    {
        insane = 8;
        rigid.spin.SetToZero();
        return;
    }

    rigid.pos += rigid.vel;

    /*
    Quat4r rotation = (0.5 * Quat4r(0, rigid.spin.x * rigid.inv_inertia.x,
                                       rigid.spin.y * rigid.inv_inertia.y,
                                       rigid.spin.z * rigid.inv_inertia.z)) * rigid.orient;
    /*/
    Quat4r rotation = (0.5 * Quat4r(0, rigid.spin.x, rigid.spin.y, rigid.spin.z)) * rigid.orient;
    //*/
    rigid.orient += rotation;
    //App::console << "DoFrame2(rigids)" << std::endl;
    //App::console << "  spin: " << rigid.spin << std::endl;
    //App::console << "  delta_rot: " << delta_rot << std::endl;
    //App::console << "  quat: " << rigid.rot << std::endl;
    rigid.orient.Normalize();
    //App::console << "  quat: " << rigid.rot << std::endl;
}

void Physics::Move(const Vec3r &offset)
{
    {
        PhyPoint *it = points;
        PhyPoint *end = points + (points_count + nodes_count);

        while (it != end)
        {
            it->pos += offset;
            ++it;
        }
    }

    PhyRigid *it = rigids;
    PhyRigid *end = rigids + rigids_count;

    while (it != end)
    {
        it->pos += offset;
        ++it;
    }
}

void Physics::Rotate(const Quat4r &offset)
{
    Mat3x3r rot(offset);

    {
        PhyPoint *it = points;
        PhyPoint *end = points + (points_count + nodes_count);

        while (it != end)
        {
            it->pos = rot * it->pos;
            ++it;
        }
    }

    PhyRigid *it = rigids;
    PhyRigid *end = rigids + rigids_count;

    while (it != end)
    {
        it->pos = rot * it->pos;
        it->orient = offset * it->orient;
        it->orient.Normalize();
        ++it;
    }
}
