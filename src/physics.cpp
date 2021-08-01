#include "pch.hpp"

#include "physics.hpp"
//#include "logging.hpp"


Physics::Physics():
    gravity(0,0,0),
    timestep(0.001),
    timestep_squared(timestep * timestep),
    break_limit(0.05),
    floor_k(10000 * timestep_squared),
    floor_d(10 * timestep),
    floor_friction(1.0),
    insane(0)
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

void Physics::UpdateForces()
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

    DO(UpdateSound, sounds, 0, sounds_count)

    // uppdatera noderna
    //DO(DoFrame0, rigids, 0, rigids_count)

    // räkna ut linjernas krafter på deras punkter
    //assert(springs != 0 || springs_count == 0);
    DO(UpdateForces, springs, 0, springs_count)

    // Räkna ut ballongernas kraft på deras punkter
    DO(UpdateForces, balloons, 0, balloons_count)

    // Låt motorerna arbeta
    DO(UpdateForces, motors, 0, motors_count)

    // håll ihop saker
    DO(UpdateForces, joints, 0, joints_count)
    //DO(DoFrame1, joints, 0, joints_count)
}

void Physics::UpdateVelocitiesAndPositions()
{
    profiler.SkipTime();

    if (insane)
        return;

    // Compute new acceleration and velocity
    DO(UpdateVelocity, points, 0, points_count)
    DO(UpdateVelocity, rigids, 0, rigids_count)

    // Compute new positions
    DO(UpdatePosition, points, 0, points_count)
    DO(UpdatePositionAndOrientation, rigids, 0, rigids_count)

    // uppdatera noderna
    DO(UpdatePointsFromRigid, rigids, 0, rigids_count)

    // Update both velocity and positions of stiffs
    DO(UpdateStiff, stiffs, 0, stiffs_count)

    REAL infinity = std::numeric_limits<REAL>::infinity();
    bounds_max = Vec3r(-infinity, -infinity, -infinity);
    bounds_min = Vec3r(infinity, infinity, infinity);

    DO(UpdateBounds, points, 0, points_count+nodes_count)
}

#undef REMEMBER_TIME
#undef DO

inline void Physics::UpdateVelocity(PhyPoint &point)
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

    const Vec3r new_acc = point.force * point.inv_mass + gravity;
    point.vel += point.acc * 0.5 + new_acc * 0.5;
    point.acc = new_acc;

    // nollställ krafter
    point.force.SetToZero();
}

inline void Physics::UpdatePosition(PhyPoint &point)
{
    // uppdatera position
    //point.pos += time * point.vel;
    point.pos += point.vel + point.acc * 0.5;
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
    UpdateBounds(point.pos);
}

inline void Physics::UpdateForces(PhySpring &spring)
{
    if (spring.broken)
        return;

    // NOTE: vel har inte dimensionen L/T utan bara L så time ska inte vara med
    Vec3r AB2(spring.p2->pos - spring.p1->pos);

    // avståndet mellan punkterna
    spring.rl = AB2.FastLength();

    // undvik division med 0
    if ( 0 == spring.rl )
    {
        //App::console << "premature return DoFrame1(spring)" << std::endl;
        return;
    }

    AB2 /= spring.rl;

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
    Vec3r velAB(spring.p2->vel - spring.p1->vel);
    Vec3r forceVec = AB2 * ( force + AB2*velAB * spring.d );

    spring.p1->force += forceVec;
    spring.p2->force -= forceVec;
}

inline void Physics::UpdateForces(PhyJoint &joint)
{
    if (joint.broken)
        return;

    Vec3r AB(joint.p2->pos - joint.p1->pos);// + joint.p2->vel - joint.p1->vel);
    Vec3r velAB(joint.p2->vel - joint.p1->vel);

    REAL length = AB.FastLength();
    /*
    if (length > break_limit) // BREAK
    {
        joint.broken = true;
        return;
    }
    */
    Vec3r forceVec(AB * joint.k + velAB * joint.d);
    if (length > 0.0001)
        forceVec += AB * (joint.s / length);

    joint.p1->force += forceVec;
    joint.p2->force -= forceVec;
}

inline void Physics::UpdateForces(PhyBalloon &balloon)
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

inline void Physics::UpdateForces(PhyMotor &motor)
{
    const Mat3x3r R_world_from_local(motor.r1->R_world_from_local);
    const Vec3r torque = R_world_from_local * motor.torque;
    motor.r1->torque -= torque;
    motor.r2->torque += torque;
}

inline void Physics::UpdateSound(PhySound &sound)
{
    sound.buffer[sound.i] = sound.p1->vel;
    sound.i = (sound.i+1) % PHY_SOUND_SAMPLES;
}

void Physics::UpdatePointsFromRigid(PhyRigid &rigid)
{
    const Mat3x3r R_world_from_local(rigid.R_world_from_local);
    const PhyNode *node = rigid.nodes;
    PhyPoint *point = rigid.points;
    const PhyPoint *end = point + rigid.nodes_count;

    while (point != end)
    {
        point->pos = R_world_from_local * node->pos; // glöm inte  att addera pos
        point->vel = rigid.vel + (rigid.spin).Cross(point->pos);
        point->pos += rigid.pos; // glöm inte  att addera pos
        point++;
        node++;
    }
}

void Physics::InitializeStiff(PhyStiff &stiff)
{
    const Mat3x3r orientationMatrix(stiff.orient);
    const REAL inv_point_mass = stiff.inv_mass * stiff.nodes_count;
    PhyNode *node = stiff.nodes;
    PhyPoint *point = stiff.points;
    PhyPoint *end = point + stiff.nodes_count;

    while (point != end)
    {
        point->pos = orientationMatrix * node->pos + stiff.pos;
        point->inv_mass = inv_point_mass;
        point++;
        node++;
    }
}

void Physics::UpdateStiff(PhyStiff &stiff)
{
    PhyNode *node = stiff.nodes;
    PhyPoint *point = stiff.points;
    PhyPoint *end = point + stiff.nodes_count;

    Vec3r mean_point_pos(0,0,0);
    Vec3r mean_node_pos(0,0,0);
    while (point != end)
    {
        mean_point_pos += point->pos;
        mean_node_pos += node->pos;
        point++;
        node++;
    }
    mean_point_pos /= stiff.nodes_count;
    mean_node_pos /= stiff.nodes_count;

    using namespace Eigen;
    Matrix3d Apq = Matrix3d::Zero();
    Matrix3d Aqq = Matrix3d::Zero();
    //Mat3x3r Apq(0,0,0,0,0,0,0,0,0);
    //Mat3x3r Aqq(0,0,0,0,0,0,0,0,0);
    point = stiff.points;
    node = stiff.nodes;
    while (point != end)
    {
        const Vector3d p = {
            point->pos.x - mean_point_pos.x,
            point->pos.y - mean_point_pos.y,
            point->pos.z - mean_point_pos.z,
        };
        const Vector3d q = {
            node->pos.x - mean_node_pos.x,
            node->pos.y - mean_node_pos.y,
            node->pos.z - mean_node_pos.z,
        };
        Apq += p * q.transpose();
        Aqq += q * q.transpose();
        point++;
        node++;
    }
    Aqq = Aqq.inverse().eval();
    SelfAdjointEigenSolver<Matrix3d> es(Apq.transpose() * Apq);
    // Matrix3d S = es.operatorSqrt();
    Matrix3d R = Apq * es.operatorSqrt().inverse();
    Matrix3d A = Apq * Aqq;
    Mat3x3r G;
    Map<Matrix3d>(G.vec1.e) =
        A * (stiff.beta * pow(A.determinant(), -1.0 / 3.0)) +
        R * (1.0 - stiff.beta);

    point = stiff.points;
    node = stiff.nodes;
    while (point != end)
    {
        const Vec3r q = {
            node->pos.x - mean_node_pos.x,
            node->pos.y - mean_node_pos.y,
            node->pos.z - mean_node_pos.z,
        };
        const Vec3r g = G * q + mean_point_pos;
        point->acc =
            stiff.alpha * (g - point->pos) +
            point->force * point->inv_mass + gravity;
        point->vel += point->acc;
        point->pos += point->vel;
        point->force.SetToZero();
        point++;
        node++;
    }
}

inline void Physics::UpdateVelocity(PhyRigid &rigid)
{
    PhyPoint *point = rigid.points;
    const PhyPoint *end = point + rigid.nodes_count;
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
    const Vec3r new_acc = rigid.force * rigid.inv_mass + gravity;
    rigid.vel += rigid.acc * 0.5 + new_acc * 0.5;
    rigid.acc = new_acc;
    rigid.force.SetToZero();
}

inline void Physics::UpdatePositionAndOrientation(PhyRigid &rigid)
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

    rigid.pos += rigid.vel + rigid.acc * 0.5;

    // Computation of rotations are based on:
    // Rozmanov, Dmitri & Kusalik, Peter. (2010).
    // Robust rotational-velocity-Verlet integration methods.
    // Physical review. E, Statistical, nonlinear, and soft matter physics.
    // 81. 056706. 10.1103/PhysRevE.81.056706.

    // 8) Perform the second half-step of rotation.
    rigid.angular_momentum += 0.5 * rigid.torque;

    // 1) Convert angular momentum and torque to local frame for t = 0

    // TODO: Avoid redundant computations.
    const Vec3r angular_momentum_in_local_t0 = Conjugate(rigid.R_world_from_local) * rigid.angular_momentum;
    const Vec3r torque_in_local_t0           = Conjugate(rigid.R_world_from_local) * rigid.torque;

    // 2) Estimate angular_momentum_in_local at t = Δt / 2

    const Vec3r angular_velocity_in_local_t0 = rigid.inv_inertia.ElemMult(angular_momentum_in_local_t0);

    const Vec3r d_dt_angular_momentum_in_local_t0 =
        torque_in_local_t0 - angular_velocity_in_local_t0.Cross(angular_momentum_in_local_t0);

    Vec3r angular_momentum_in_local_t_halfway =
        angular_momentum_in_local_t0 + 0.5 * d_dt_angular_momentum_in_local_t0;

    // 3) Zeroth approximation of time derivative of R_world_from_local at t = Δt/2

    Quat4r d_dt_R_world_from_local_t_halfway =
        Quat4r(0.0, 0.5 * rigid.R_world_from_local * rigid.inv_inertia.ElemMult(angular_momentum_in_local_t_halfway));

    // 4) Zeroth approximation of R_world_from_local at t = Δt/2

    Quat4r R_world_from_local_t_halfway = rigid.R_world_from_local + 0.5 * d_dt_R_world_from_local_t_halfway;
    R_world_from_local_t_halfway.Normalize();

    // 5) Propagate angular_momentum_in_world

    rigid.angular_momentum += 0.5 * rigid.torque;

    // 6) Iterate over k until |R_world_from_local[k+1](t=Δt/2) - R_world_from_local[k](t=Δt/2)| < epsilon, eg. epsilon = 10^-9

    for(int i = 0; i < 8; ++i)
    {
        angular_momentum_in_local_t_halfway =
            Conjugate(R_world_from_local_t_halfway) * rigid.angular_momentum;

        const Vec3r angular_velocity_in_local_t_halfway =
            rigid.inv_inertia.ElemMult(angular_momentum_in_local_t_halfway);

        d_dt_R_world_from_local_t_halfway =
            0.5 * R_world_from_local_t_halfway * Quat4r(0.0, angular_velocity_in_local_t_halfway);

        R_world_from_local_t_halfway = rigid.R_world_from_local + 0.5 * d_dt_R_world_from_local_t_halfway;
        R_world_from_local_t_halfway.Normalize();
    }

    // 7) When d/dt R_world_from_local(t=Δt/2) is found, we can compute R_world_from_local(t=Δt) and torque at t = Δt

    rigid.R_world_from_local += d_dt_R_world_from_local_t_halfway;
    rigid.R_world_from_local.Normalize();

    rigid.torque.SetToZero();

    const Vec3r angular_velocity_in_local_t_halfway =
        rigid.inv_inertia.ElemMult(angular_momentum_in_local_t_halfway);
    rigid.spin = R_world_from_local_t_halfway * angular_velocity_in_local_t_halfway;
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
        it->R_world_from_local = offset * it->R_world_from_local;
        it->R_world_from_local.Normalize();
        ++it;
    }
}
