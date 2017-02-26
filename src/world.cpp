#include <sstream>
#include <GL/glew.h>
//#include "SDL_opengl.h"

//#include "cgstruct.h"
#include "logging.hpp"
#include "parser.hpp"
#include "physfsrwops.h"
#include "physics.hpp"
#include "physstruct.hpp"
#include "player.hpp"
#include "world.hpp"

using namespace std;

namespace App
{
	World *world;
} // namespace App

World::World(const char filename[])
{
    LOG_IF_ERROR("Start of World::World(...)");

    sky_displaylist = 0;

    //tiles = NULL;
    heights = NULL;
    size.x = 1;
    size.y = 1;
    size.z = 1;
    pos.SetToZero();
    res_x = 0;
    res_y = 0;
    Parser parser;

    if ( 0==parser.Load(filename) )
    {
        LOG_S(ERROR) << "World::World(): failed to open world \"" << filename << "\"";
        return;
    }

    TypeName typeName;

    typeof(parser.lines.end()) it;
    for (it = parser.lines.begin(); it != parser.lines.end(); ++it)
    {
        stringstream ss;
        ss << *it;

        if ( !(ss >> typeName.type) )
            continue;

        if (typeName.type == "texture")
        {
            if (ss >> typeName.name)
                texture.Aquire( typeName.name.c_str() );
        }
        else if (typeName.type == "heightmap")
        {
            // TODO: Avoid loading the image twice
            if (ss >> typeName.name)
                heightmap.Aquire( typeName.name.c_str() );
            BuildHeightMap(typeName.name.c_str());
            LOG_IF_ERROR("heightmap");
        }
        else if (typeName.type == "normalmap")
        {
            if (ss >> typeName.name)
                normalmap.Aquire( typeName.name.c_str() );
            LOG_IF_ERROR("normalmap");
        }
        else if (typeName.type == "decalmap")
        {
            if (ss >> typeName.name)
                decalmap.New( typeName.name.c_str() );
            LOG_IF_ERROR("decalmap");
        }
        else if (typeName.type == "skybox")
        {
            if (ss >> typeName.name)
            {
                sky.Aquire( typeName.name.c_str() );
                BuildSky();
            }
            LOG_IF_ERROR("skybox");
        }
        else if (typeName.type == "size")
        {
            ss >> size;
        }
        else if (typeName.type == "pos")
        {
            ss >> pos;
        }
        else if (typeName.type == "player_pos")
        {
            ss >> App::player.pos;
        }
        else if (typeName.type == "player_rot")
        {
            Vec3r deg;
            ss >> deg;
            if ( !ss.fail() )
            {
                Quat4r rot =
                    Quat4r( deg.x*(M_PI/180.0), Vec3r(1,0,0) ) *
                    Quat4r( deg.y*(M_PI/180.0), Vec3r(0,1,0) ) *
                    Quat4r( deg.z*(M_PI/180.0), Vec3r(0,0,1) );
                rot.Normalize();
                App::player.rot = rot;
            }
        }
        else if (typeName.type == "dynamic")
        {
            ss >> typeName.name;
            // TODO(Rasmus): Make a more proper text of filename extension.
            PhyInstance *inst = ( typeName.name.back() == 'l' ?
                PhyInstance::InsertPhysXML(typeName.name.c_str()) :
                PhyInstance::InsertPhysJSON(typeName.name.c_str()) );
            if (inst == NULL)
            {
                LOG_S(ERROR) << "World::World(): failed to load \"" << typeName.name << "\"";
                continue;
            }

            while ( !(ss >> typeName.type).fail() )
            {
                // FIXME: This should not be order dependant, or should it?
                if (typeName.type == "pos")
                {
                    Vec3r pos;
                    ss >> pos;
                    if ( !ss.fail() )
                        inst->phys->Move(pos);
                }
                else if (typeName.type == "rot")
                {
                    Vec3r deg;
                    ss >> deg;
                    if ( !ss.fail() )
                    {
                        Quat4r rot =
                            Quat4r( deg.x*(M_PI/180.0), Vec3r(1,0,0) ) *
                            Quat4r( deg.y*(M_PI/180.0), Vec3r(0,1,0) ) *
                            Quat4r( deg.z*(M_PI/180.0), Vec3r(0,0,1) );
                        VLOG_S(1) << "Rotation: " << rot;
                        rot.Normalize();
                        VLOG_S(1)  << "Normalized: " << rot;
                        inst->phys->Rotate(rot);
                    }
                }
            }
        }
    }

    LOG_IF_ERROR("End of World::World(...)");
}

World::~World()
{
	/*
    if ( glIsTexture(texture) )
        glDeleteTextures(1, &texture); // ignorerar om texture är 0
    */
    //delete [] tiles;
    //tiles = NULL;
    delete [] heights;
    heights = NULL;

    if ( sky_displaylist != 0 )
    {
        glDeleteLists(sky_displaylist, 1);
    }
}

void World::SaveTo(const char p_filename[])
{
/*
    ofstream file(p_filename);
    file << size.x << " " << size.y << "\n";

    for (int i = 0; i < size.x*size.y; i++)
    {
        file << tiles[i];
    }
*/
}

void World::Clear()
{
/*
    if ( tiles )
        memset( tiles, 0, int(size.x*size.y*sizeof(Tiles)) );
*/
}

template <class T>
T Lerp(const T &a, const T &b, float f)
{
    //f = Clamp(f, 0.0f, 1.0f);
    return a*(1.0f-f) + b*f;
};

// TODO: Optimize DRAWQUADS a lot
// creating a display list for the whole cube could be a good idea
#define DRAWQUADS(x,xu,xv, y,yu,yv, z,zu,zv, skyres) \
{ \
    float scale = 1.0 / skyres; \
    float X,Y,Z,u,v; \
    Vec3<float> normal; \
    for (unsigned int i=1; i<=skyres; ++i) \
    { \
        glBegin(GL_QUAD_STRIP); \
        for (unsigned int j=0; j<=skyres; ++j) \
        { \
            u = j*scale; \
            v = i*scale; \
            X = (x)+(xu)*u+(xv)*v; \
            Y = (y)+(yu)*u+(yv)*v; \
            Z = (z)+(zu)*u+(zv)*v; \
            glTexCoord2f(u, v); \
            normal = Vec3<float>(-X,-Y,-Z); \
            normal.Normalize(); \
            glNormal3fv(normal.e); \
            glVertex3f(X, Y, Z); \
            v -= scale; \
            X = x+xu*u+xv*v; \
            Y = y+yu*u+yv*v; \
            Z = z+zu*u+zv*v; \
            glTexCoord2f(u, v); \
            normal = Vec3<float>(-X,-Y,-Z); \
            normal.Normalize(); \
            glNormal3fv(normal.e); \
            glVertex3f(X, Y, Z); \
        } \
        glEnd(); \
    } \
}

void DrawQuads(float x,float xu,float xv, float y,float yu,float yv, float z,float zu,float zv,float skyres)
{
    float scale = 1.0 / skyres;

    float X,Y,Z,u,v;
    for (unsigned int i=1; i<=skyres; ++i)
    {
        glBegin(GL_QUAD_STRIP);
        for (unsigned int j=0; j<=skyres; ++j)
        {
            u = j*scale;
            v = i*scale;
            X = (x)+(xu)*u+(xv)*v;
            Y = (y)+(yu)*u+(yv)*v;
            Z = (z)+(zu)*u+(zv)*v;
            //glTexCoord2f(u, v);
            glTexCoord3f(X, Y, Z);
            //glTexCoord3f(Y, Z, X);
            //normal = Vec3<float>(-X,-Y,-Z);
            //normal.Normalize();
            //glNormal3fv(normal.e);
            glVertex3f(X, Y, Z);
            v -= scale;
            X = x+xu*u+xv*v;
            Y = y+yu*u+yv*v;
            Z = z+zu*u+zv*v;
            //glTexCoord2f(u, v);
            glTexCoord3f(X, Y, Z);
            //glTexCoord3f(Y, Z, X);
            //normal = Vec3<float>(-X,-Y,-Z);
            //normal.Normalize();
            //glNormal3fv(normal.e);
            glVertex3f(X, Y, Z);
        }
        glEnd();
    }
}

void World::BuildSky()
{
    if ( sky_displaylist == 0 )
        sky_displaylist = glGenLists(1);

    glNewList(sky_displaylist, GL_COMPILE);

    float skyres = 12;

    glDepthFunc(GL_LEQUAL);
    glDisable(GL_DEPTH_TEST);
    //*
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
    //*/
    // TODO: Disable depth-test and lighting
    // The sky could be drawn first with depth-writing off,
    // but there might be a performance gain if drawn last.
    // Lighting could be disabled by using multitexture functions.
    //glDisable(GL_DEPTH_TEST);

    //glEnable(GL_AUTO_NORMAL);

    //glPushMatrix();
    //glLoadIdentity();
    //glScalef(20, 20, 20);
    //App::cg.UpdateMatrixParameters();

    sky.Enable();
    sky.Bind();

    //GLuint mode = GL_NORMAL_MAP_EXT;
    //GLuint mode = GL_REFLECTION_MAP_EXT;

    // -x, -y, +x
    // -z, +z, +y
    //sky_left.Bind();
    glNormal3f(1, 0, 0);
    DrawQuads(-10,0,0, -10,20,0, -10,0,20, skyres);

    //sky_back.Bind();
    glNormal3f(0, 1, 0);
    DrawQuads(10,-20,0, -10,0,0, -10,0,20, skyres);

    //sky_right.Bind();
    glNormal3f(-1, 0, 0);
    DrawQuads(10,0,0, 10,-20,0, -10,0,20, skyres);

    //sky_down.Bind();
    glNormal3f(0, 0, 1);
    DrawQuads(-10,20,0, -10,0,20, -10,0,0, skyres);

    //sky_top.Bind();
    glNormal3f(0, 0, -1);
    DrawQuads(-10,20,0, 10,0,-20, 10,0,0, skyres);

    //sky_front.Bind();
    glNormal3f(0, -1, 0);
    DrawQuads(-10,20,0, 10,0,0, -10,0,20, skyres);

    //glPopMatrix();
    //App::cg.UpdateMatrixParameters();

    //glDisable(GL_AUTO_NORMAL);


    glEnable(GL_DEPTH_TEST);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDepthFunc(GL_LEQUAL);

    sky.Disable();

    glEndList();
}

void World::DrawSky()
{
    if (sky_displaylist != 0)
        glCallList(sky_displaylist);
}

void World::Draw()
{
    LOG_IF_ERROR("Start of World::Draw()");

    glPushMatrix();
#ifdef REAL_DOUBLE
    glTranslated( App::player.pos.x, App::player.pos.y, App::player.pos.z );
#else
    glTranslatef( App::player.pos.x, App::player.pos.y, App::player.pos.z );
#endif

    DrawSky();

    glPopMatrix();

    //return;

    if (!heights)
        return;

    float scale_x = 1.0 / (res_x-1);
    float scale_y = 1.0 / (res_y-1);

    glPushMatrix();

#ifdef REAL_DOUBLE
    glTranslated( pos.x, pos.y, pos.z );
    glScaled( size.x/(res_x-1), size.y/(res_y-1), size.z );
#else
    glTranslatef( pos.x, pos.y, pos.z );
    glScalef( size.x/(res_x-1), size.y/(res_y-1), size.z );
#endif


    glActiveTextureARB(GL_TEXTURE0_ARB);
    texture.Enable();
    texture.Bind();
    LOG_IF_ERROR("Bind#1")

    if (decalmap.GetID() != 0)
    {
        // Instuct opengl to mix texture and decal layer
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_COMBINE);
        glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB, GL_INTERPOLATE);
        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB, GL_TEXTURE1);
        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB, GL_TEXTURE0);
        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE2_RGB, GL_TEXTURE1);
        glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB, GL_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB, GL_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND2_RGB, GL_SRC_ALPHA);
        LOG_IF_ERROR("glTexEnv#1 COLOR")

        glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_ALPHA, GL_ADD);
        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_ALPHA, GL_TEXTURE1);
        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_ALPHA, GL_TEXTURE0);
        glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
        glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
        LOG_IF_ERROR("glTexEnv#1 ALPHA")

        glActiveTextureARB(GL_TEXTURE1_ARB);
        //glClientActiveTextureARB(GL_TEXTURE1_ARB);

        decalmap.Enable();
        decalmap.Bind();
        LOG_IF_ERROR("Bind#2")

        // Instruct opengl to blend the combined texture with the lighting calculations
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_COMBINE);
        glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB, GL_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB, GL_SRC_COLOR);

        LOG_IF_ERROR("glTexEnvi#2")
    }

    glColor3f(1, 1, 1);
    glNormal3f(0,0,1);

    for (unsigned int i=1; i<res_y; ++i)
    {
        glBegin(GL_QUAD_STRIP);
        for (unsigned int j=0; j<res_x; ++j)
        {
            float u = j*scale_x;
            float v = i*scale_y;

            glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);
            glMultiTexCoord2fARB(GL_TEXTURE1_ARB, u, v);
            //glTexCoord2f( j*scale_x, i*scale_y );
            glVertex3f( j, i, heights[i*res_x + j] );

            v -= scale_y;
            glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);
            glMultiTexCoord2fARB(GL_TEXTURE1_ARB, u, v);
            //glTexCoord2f( j*scale_x, (i-1)*scale_y );
            glVertex3f( j, i-1, heights[(i-1)*res_x + j] );
        }
        glEnd();
    }

    if (decalmap.GetID() != 0)
    {
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_MODULATE);
        decalmap.Disable();
    }

    glActiveTextureARB(GL_TEXTURE0_ARB);
    //glClientActiveTextureARB( GL_TEXTURE0_ARB );
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_MODULATE);
    texture.Disable();

    glPopMatrix();
    LOG_IF_ERROR("world end")

    //App::cg.UpdateMatrixParameters();
}

void World::BuildHeightMap(const char *filename)
{
    SDL_RWops *rw = PHYSFSRWOPS_openRead(filename);

    if (rw == NULL)
    {
        LOG_S(ERROR) << "IMG_Load failed: " << PHYSFS_getLastError();
        return;
    }

    SDL_Surface *image = IMG_Load_RW(rw, 1);

    if (!image)
    {
        LOG_S(ERROR) << "IMG_Load failed: " << SDL_GetError();
        return;
    }

    res_x = image->w;
    res_y = image->h;

    delete [] heights;
    heights = new float[res_x*res_y];

    // TODO: Choose the right colour channel depending on the image
    int size = (image->pitch/res_x);

    for (unsigned int i=0; i<res_y; ++i)
        for (unsigned int j=0; j<res_x; ++j)
        {
            unsigned int pos = (res_y-1-i)*image->pitch + j*size;
            heights[i*res_x + j] = ((unsigned char*)image->pixels)[pos]*(1.0/255);
        }

    SDL_FreeSurface(image);
}

void World::CollideWorld(Physics &phys)
{
    // Calculate highest terrain value within phys x and y bounds
    REAL a,b,c,d;
    a = Clamp<REAL>((phys.bounds_min.x-pos.x)/size.x, 0.0, 1.0) * (res_x-1);
    b = Clamp<REAL>((phys.bounds_min.y-pos.y)/size.y, 0.0, 1.0) * (res_y-1);
    c = Clamp<REAL>((phys.bounds_max.x-pos.x)/size.x, 0.0, 1.0) * (res_x-1);
    d = Clamp<REAL>((phys.bounds_max.y-pos.y)/size.y, 0.0, 1.0) * (res_y-1);

    int x1 = int(floor(a));
    int x2 = int(ceil(c));
    int y1 = int(floor(b)) * res_x;
    int y2 = int(ceil(d)) * res_x;

    REAL max_height = heights[x1 + y1];
    for (int i = y1; i <= y2; i += res_x)
    {
        for (int j = x1; j <= x2; ++j)
        {
            REAL h = heights[i + j];
            if (h > max_height)
                max_height = h;
        }
    }

    // NOTE: HACK!!
    max_height = size.z*max_height + pos.z + 0.25;

    // if phys bounds is above max terrain value, no collision can occur
    //FIXME: is this true when the object is falling towards the terrain?
    if (phys.bounds_min.z > max_height)
        return;

    // Test all points agains the terrain
    PhyPoint *it = phys.points;
    PhyPoint *end = phys.points + phys.points_count;
    for (;it != end; ++it)
    {
        Vec3r pos2 = it->pos + it->vel;

        if (pos2.z > max_height)
            continue;

        // NOTE: HACK!!
        REAL height = GetHeight(pos2.x, pos2.y);
        pos2.z -= height;
        if ( pos2.z < 0 )
        {
            Vec3r normal = GetNormal(pos2.x, pos2.y, 0.1);

            // tangential friction
            Vec3r delta = it->vel - normal * (normal * it->vel);
            REAL friction = it->vel * normal * -10;

            if ( delta.SqrLength() > friction*friction )
                delta = Normalize(delta) * fabs(friction);

            it->vel -= delta;

            // displacement
            it->vel -= normal * (pos2.z / normal.z);
        }
    }

    // Test rigids
    end = phys.points + (phys.points_count + phys.nodes_count);
    for (;it != end; ++it)
    {
        REAL z2 = it->pos.z + it->vel.z;

        if (z2 > max_height)
            continue;

        // NOTE: HACK!!
        z2 -= GetHeight(it->pos.x, it->pos.y) + 0.25;

        if ( z2 < 0 )
        {
            Vec3r normal = GetNormal(it->pos.x, it->pos.y, 0.1);

            REAL force;
            force = -z2*1000*phys.time*phys.time; // spring k

            //if (it->vel.z < 0)
            //    force -= it->vel.z*10*phys.time; // spring d

            // tangential friction
            Vec3r vec = it->vel/phys.time;
            vec -= normal * (vec * normal);
            //vec += Normalize(vec)*2.0;

            it->force -= vec*0.3*force;

            it->force += normal * force;
        }
    }
}
