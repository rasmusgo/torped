#pragma once

#include <GL/glew.h>
#include <string>
#include <cmath>
#include <SDL.h>
#include "SDL_image.h"
//#include "SDL_opengl.h"
//#include <gl/gl.h>
//#include <gl/glext.h>

#include "texture.hpp"
#include "vectormath.hpp"
//float g_model1[6 * ]

class Physics;

struct Tiles
{
	char model :	4;
	char type :		3;
	char item :		1;
	float height;
	friend std::ostream& operator << (std::ostream &os, const Tiles &a)
	{
		for (unsigned int i=0; i<sizeof(a); i++)
            os << ((char*)(&a))[i];
		return os;
	};
	friend std::istream& operator >> (std::istream &is, Tiles &a)
	{
		for (unsigned int i=0; i<sizeof(a); i++)
            is >> ((char*)(&a))[i];
		return is;
	};
};

class World
{
public:
	World(const char filename[]);
	~World();
	void SaveTo(const char filename[]);
	void Clear();
	REAL GetHeight(REAL x, REAL y)
    {
        REAL a,b;
        a = Clamp<REAL>((x-pos.x)/size.x, 0.0, 1.0) * (res_x-1);
        b = Clamp<REAL>((y-pos.y)/size.y, 0.0, 1.0) * (res_y-1);

        int x1 = int(floor(a));
        int x2 = int(ceil(a));
        int y1 = int(floor(b)) * res_x;
        int y2 = int(ceil(b)) * res_x;

        REAL lerpx = fmod(a, 1);
        REAL lerpy = fmod(b, 1);

        REAL h1 = heights[x1 + y1];
        REAL h2 = heights[x2 + y1];
        REAL h3 = heights[x1 + y2];
        REAL h4 = heights[x2 + y2];

        REAL h5 = (h2-h1)*lerpx + h1;
        REAL h6 = (h4-h3)*lerpx + h3;

        return size.z*( (h6-h5)*lerpy + h5 ) + pos.z;
        /*
        return size.z*heights[(int)floor(a * res_x + b)] + pos.z;

        return size.z*heights[ (int)floor(int(Clamp((y-pos.y)/size.y, 0.0, 1.0)*(res_y-1))*res_x + Clamp((x-pos.x)/size.x, 0.0, 1.0) * (res_x-1)) ] + pos.z;
        */
    }
	Vec3r GetNormal(REAL x, REAL y, const REAL h=0.1)
	{
        REAL a,b;
        a = Clamp<REAL>((x-pos.x)/size.x, 0.0, 1.0) * (res_x-1);
        b = Clamp<REAL>((y-pos.y)/size.y, 0.0, 1.0) * (res_y-1);

        unsigned int x1 = (unsigned int)(floor(a));
        unsigned int x2 = x1 + 1;
        unsigned int y1 = (unsigned int)(floor(b)) * res_x;
        unsigned int y2 = y1 + res_x;

        if (x2 > res_x-1)
            x2 = res_x-1;
        if (y2 > res_x*(res_y-1))
            y2 = res_x*(res_y-1);

        REAL h1 = heights[x1 + y1];
        REAL h2 = heights[x2 + y1];
        REAL h3 = heights[x1 + y2];
        REAL h4 = heights[x2 + y2];

        Vec3r vec1(size.x/(res_x-1), size.y/(res_y-1), size.z*(h4-h1));
        Vec3r vec2(size.x/(res_x-1), -size.y/(res_y-1), size.z*(h2-h3));
        return vec2.UnitCross( vec1 );
	}
	void Draw();
	void DrawSky();
	void BuildSky();
	void BuildHeightMap(const char *filename);
    void CollideWorld(Physics &phys);
//private:
/*
	REAL size_x;
	REAL size_y;
	REAL size_z;
*/
    Vec3r size;
	Vec3r pos;
	float *heights;
	unsigned int res_x, res_y;
	Texture texture;
	Texture heightmap;
	Texture normalmap;
	Texture decalmap;
    Texture sky;
    GLuint sky_displaylist;
};

namespace App
{
	extern World *world;
} // namespace App
