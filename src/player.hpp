#pragma once

#include "vectormath.hpp"

class Player
{
public:
	Player();
	void MoveMouse(float x, float y);
	void MoveArm1(float x, float y);
	void MoveArm2(float x, float y);
	void Move(float x, float y, float z=0);
	void Fly(float x, float y, float z=0);
	void Do(const char cmd[]);
//private:
	Vec3r pos;
	Vec3r vel;

    Quat4r rot;
    Vec3r arm1, arm2;
};

namespace App
{
	extern Player player;
}
