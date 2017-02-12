#include "alstruct.hpp"
#include "logging.hpp"
#include "physstruct.hpp"
#include "player.hpp"

namespace App
{
	Player player;
}

Player::Player():
	pos(0,0,0), vel(0,0,0),
	//zrot(0), zrot2(0), xrot(0), xrot2(0)
	rot(1,0,0,0), arm1(0,0,0), arm2(0,0,0)
{

}

void Player::MoveArm1(float x, float y)
{
    arm1.x += x;
    arm1.y += y;

    Vec3r mean = (arm1 + arm2) * 0.5;
    arm1 -= mean;
    arm2 -= mean;

    MoveMouse(mean.x, mean.y);

    //Mat3x3r mat(rot);
    //arm1 += mat*Vec3r(x,0,y);
}

void Player::MoveArm2(float x, float y)
{
    arm2.x += x;
    arm2.y += y;

    Vec3r mean = (arm1 + arm2) * 0.5;
    arm1 -= mean;
    arm2 -= mean;

    MoveMouse(mean.x, mean.y);

    //Mat3x3r mat(rot);
    //arm2 += mat*Vec3r(x,0,y);
}

void Player::MoveMouse(float x, float y)
{
    const REAL scale = M_PI/1800.0;
    Quat4r q_x( 0.5*x*scale, Vec3r(0,0,-1) );
    //q_x.Normalize();
    Quat4r q_y( y*scale, Vec3r(0,1,0) );
    //q_y.Normalize();
    rot = q_x*rot*q_x*q_y;
    rot.Normalize();

    //if ( fabs(mat.vec1.z) < 0.75)
    {
        //Mat3x3r mat(rot);
        Vec3r euler = Euler(rot);
        REAL limit = Clamp( fabs(euler.y)*6.0-1.0*M_PI,
                            0.0, std::numeric_limits<double>::infinity() );
        euler.x = Clamp(euler.x, -limit, limit);
        rot = Quat4r( euler.z, Vec3r(0,0,1) ) *
              Quat4r( euler.y, Vec3r(0,1,0) ) *
              Quat4r( euler.x, Vec3r(1,0,0) );

        rot.Normalize();
    }

    Mat3x3r mat(rot);

    ALfloat orientation[] = { float(mat.vec1.x), float(mat.vec1.y), float(mat.vec1.z),
                              float(mat.vec3.x), float(mat.vec3.y), float(mat.vec3.z) };

    alListenerfv(AL_ORIENTATION, orientation);
}

void Player::Fly(float x, float y, float z)
{
    Mat3x3r mat(rot);

	pos += x * mat.vec1;
	pos += y * mat.vec2;
	pos += z * mat.vec3;

    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
    alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
	alGetError(); // ignore errors
}

void Player::Move(float x, float y, float z)
{
    Mat3x3r mat(rot);
    mat.vec2.z = 0;
    mat.vec2.Normalize();
    Vec3r forward = mat.vec2.UnitCross( Vec3r(0,0,1) );

	pos += x * forward;
	pos += y * mat.vec2;

	pos.z += z;

    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
    alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
	alGetError(); // ignore errors
}

void Player::Do(const char cmd[])
{
    std::stringstream ss;
    std::string str;
    ss << cmd;
    ss >> str;

    if (str == "vel")
        ss >> App::player.vel;
    else if (str == "velx")
        ss >> App::player.vel.x;
    else if (str == "vely")
        ss >> App::player.vel.y;
    else if (str == "velz")
        ss >> App::player.vel.z;
    else if (str == "stop")
        App::player.vel.SetToZero();
    else if (str == "pose")
    {
        float a,b;
        a = b = std::numeric_limits<float>::quiet_NaN();

        // if there is no pose name, the pose "pose" will be loaded
        ss >> str >> a >> b;

        std::lock_guard<std::mutex> lock(phyInstances_lock);

        if (phyInstances.empty())
        {
            VLOG_S(2) << "No physics instance";
            return;
        }

        bool fail;
        if (b == b)
            fail = !phyInstances.back()->UpdatePhysBlend(str.c_str(), a, b);
        else if (a == a)
            fail = !phyInstances.back()->UpdatePhysBlend(str.c_str(), 1.0f-a, a);
        else
            fail = !phyInstances.back()->UpdatePhys(str.c_str());

        if (fail)
        {
            // TODO: report error
        }

    }
    else if (str == "motor")
    {
        float x,y,z;
        x = y = z = std::numeric_limits<float>::quiet_NaN();

        ss >> str >> x >> y >> z;

        std::lock_guard<std::mutex> lock(phyInstances_lock);

        if (phyInstances.empty())
        {
            VLOG_S(2) << "No physics instance";
            return;
        }

        PhyInstance *inst = phyInstances.back().get();
        TypeName tn;
        tn.type = "motor";
        tn.name = str;
        if ( inst->namesIndex.find(tn) != inst->namesIndex.end() )
            inst->phys->motors[inst->namesIndex[tn]].torque = Vec3r(x,y,z) * (inst->phys->time * inst->phys->time);
    }
}
