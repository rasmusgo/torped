#include <mutex>

#include <GL/glew.h>
#include <SDL.h>
#include "SDL_thread.h"
//#include "SDL_opengl.h"
#include "SDL_getenv.h"
#include <GL/glut.h>
#include <physfs.h>
#include <deque>
#include <Python.h>
#include <angelscript.h>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "alstruct.h"
#include "aswrap.h"
#include "collide.h"
#include "comparelist.h"
#include "console.h"
#include "gameapp.h"
#include "input.h"
#include "logging.h"
#include "luawrap.h"
#include "menu.h"
#include "physfsstruct.h"
#include "player.h"
#include "pythonstruct.h"
#include "scene.h"
#include "shader.h"
#include "world.h"

#ifdef REAL_DOUBLE
#define glVertex3rv glVertex3dv
#define glNormal3rv glNormal3dv
#else
#define glVertex3rv glVertex3fv
#define glNormal3rv glNormal3fv
#endif

// this one comes from texture.cpp
void FlipImageY(SDL_Surface *image);
//PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;
//PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2fARB = NULL;

struct aPair
{
    unsigned int a, b;
};

namespace App
{
    extern std::vector<std::string> commands;
    int xRes;
    int yRes;
    int fullscreen;
    SDL_Surface *screen;
    //World *world;
    //Player player;
    AppMode appMode;
    //CgStruct cg;
    //AlStruct al;

    int developermode;
    int wireframemode;
    int stereo3dmode;
    float stereo3d_depth;
    float stereo3d_focus;
    float view_depth;
    int profilermode;

    std::vector<SDL_Thread*> threads;

    bool fatal_error = false;

    Profiler profiler;

    Shader shader;

    asIScriptEngine *as_engine = NULL;
    lua_State* lua_console = NULL;

    SDL_Thread *scene_thread = NULL;
    Scene *scene = NULL;

    std::mutex game_physics_messages_lock;

    void InitAll(int argc, char *argv[])
    {
        xRes = 1024;
        yRes = 768;
        fullscreen = 0;
        appMode = MENU;
        world = NULL;
//        font = NULL;
        developermode = 0;
        wireframemode = 0;
        stereo3dmode = 0;
        stereo3d_depth = 0.003;
        stereo3d_focus = 1.0;
        view_depth = 100;
        profilermode = 0;

        console.clear();
        commands.clear();
        commands.push_back("");
        //App::console.rdbuf( std::cout.rdbuf() );

        PHYSFS_init(argv[0]);
        std::stringstream data_path, data_archive;
        data_path << PHYSFS_getBaseDir() << "data" << PHYSFS_getDirSeparator();
        data_archive << PHYSFS_getBaseDir() << "data.rar";

        if ( !PHYSFS_addToSearchPath(data_path.str().c_str(), 0) )
        {
            LOG_S(ERROR) << "PHYSFS_addToSearchPath(\"" << data_path.str() << "\") Failed: " << PHYSFS_getLastError();
        }
        /*
        if ( !PHYSFS_addToSearchPath(data_archive.str().c_str(), 1) )
        {
            App::console << "PHYSFS_addToSearchPath(\"" << data_archive.str() << "\") Failed: " << PHYSFS_getLastError() << std::endl;
            App::FlushConsole();
        }
        */
        if ( !PHYSFS_setWriteDir(data_path.str().c_str()) )
        {
            LOG_S(ERROR) << "PHYSFS_setWriteDir(\"" << data_path.str() << "\") Failed: " << PHYSFS_getLastError();
        }

        LOG_S(INFO) << "Initializing GLUT...";

        glutInit(&argc, argv);

        LOG_S(INFO) << "Initializing SDL...";

        putenv("SDL_VIDEO_CENTERED=1");
#if 0
        if ( getenv("SDL_VIDEODRIVER") == NULL )
            //*
            putenv("SDL_VIDEODRIVER=directx"); /*/
            putenv("SDL_VIDEODRIVER=windib");
            //*/
#endif

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) != 0)
        {
            LOG_S(ERROR) << "Fatal error: SDL_Init failed: " << SDL_GetError();
            return;
        }

        LOG_S(INFO) << "Initializing video...";

        if (SDL_GL_LoadLibrary(NULL) != 0)
        {
            LOG_S(ERROR) << "Error: SDL_GL_LoadLibrary failed: " << SDL_GetError();
        }

        // icon is icon TEXT not icon filename
        SDL_WM_SetCaption("Torped", "Torped");
        /*
        SDL_Surface *icon = IMG_Load("torped.png");
        SDL_WM_SetIcon(icon, NULL);
        SDL_FreeSurface(icon);
        //*/

        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

        screen = SDL_SetVideoMode(xRes, yRes, 0, (fullscreen ? SDL_FULLSCREEN : 0) | SDL_OPENGL);
        if (screen == NULL)
        {
            LOG_S(ERROR) << "Fatal error: SDL_SetVideoMode failed: " << SDL_GetError();
            return;
        }

        int value = -1;
        SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
        VLOG_S(1) << "SDL_GL_DEPTH_SIZE: " << value;

        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            LOG_S(ERROR) << "Fatal error: glewInit failed: " << glewGetErrorString(err);
            return;
        }

        //glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

        shader.Aquire("fisheye.vert", "");
        LOG_IF_ERROR("end of video init");

        LOG_S(INFO) << "Initializing input...";

        InitInput();

        LOG_S(INFO) << "Initializing audio...";

        if (!al.InitAl())
        {
            LOG_S(ERROR) << "Fatal error: InitAl() failed";
            fatal_error = true;
            return ;
        }

        ALuint source = al.AddSound("bomb.ogg", Vec3r(3, 3, 2));

        if (!source)
        {
            LOG_S(ERROR) << "AlStruct::AddSound failed";
            return ;
        }
        //alSourcei(source, AL_LOOPING, AL_TRUE);
        alSourcePlay(source);

        LOG_S(INFO) << "Initializing Python...";

        if (!InitPython())
        {
            LOG_S(ERROR) << "Fatal error: InitPython() failed";
            fatal_error = true;
            return;
        }
        PySys_SetArgv(argc, argv);

        LOG_S(INFO) << "Initializing AngelScript...";

        // Create the script engine
        as_engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

        if( as_engine == NULL )
        {
            LOG_S(ERROR) << "Fatal error: Failed to create AngelScript engine.";
            fatal_error = true;
            return;
        }

        // Configure the script engine with all the functions,
        // and variables that the script should be able to use.
        AsWrapConsoleCmds(as_engine);
        AsRunFile(as_engine, "autoexec.as");

        // Initialize Lua
        LOG_S(INFO) << "Initializing Lua...";

        lua_console = luaL_newstate();
        if( lua_console == NULL )
        {
            LOG_S(ERROR) << "Fatal error: Lua failed to open.";
            fatal_error = true;
            return;
        }

        //luaL_openlibs(lua_console);
        LuaWrapConsoleCmds(lua_console);
        LuaRunFile(lua_console, "autoexec.lua");

        LOG_S(INFO) << "Initializing physics...";

        if (!InitPhys())
        {
            LOG_S(ERROR) << "Fatal error: InitPhys() failed";
            fatal_error = true;
            return;
        }

        LOG_S(INFO) << "Initializing Threads...";
        scene = new Scene();
        scene_thread = SDL_CreateThread(StartScene, (void*)scene);

        if ( scene_thread == NULL )
        {
            LOG_S(ERROR) << "Fatal error: Failed to create physics thread";
            fatal_error = true;
            return;
        }
        threads.push_back(scene_thread);

        LOG_S(INFO) << "Initialization finished!";

        SwitchMode(appMode);
        LOG_IF_ERROR("End of InitAll(...)")
    }

    void QuitAll()
    {
        if (scene)
        {
            scene->AddFlags(SCENE_QUIT);
        }

        // wait for all open threads
        for (unsigned int i=0; i<threads.size(); ++i )
            SDL_WaitThread(threads[i], NULL);

        threads.clear();

        delete scene;
        scene = NULL;

        for (typeof(actions_table.end()) it = actions_table.begin(); it != actions_table.end(); ++it)
        {
            delete it->second;
            it->second = NULL;
        }
        actions_table.clear();

        // Close Lua console
        if (lua_console)
            lua_close(lua_console);

        // Release the AngelScript engine
        if (as_engine)
            as_engine->Release();

        QuitPython();
        // destroy and quit physics
        QuitPhys();

        delete world;
        world = NULL;

        // shut down openal
        al.QuitAl();

        for( unsigned int i=0; i < joysticks.size(); i++ )
        {
            if (joysticks[i] != NULL)
                SDL_JoystickClose(joysticks[i]);
        }
        joysticks.clear();

        SDL_Quit();
        ManyMouse_Quit();
        LOG_S(INFO) << "shutdown";

        // this should be last if something needs to be saved
        PHYSFS_deinit();
    }

    void Quit()
    {
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);
    }

    void SetupGame()
    {
        glViewport(0, 0, xRes, yRes);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0);
        glLineWidth(1);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_TEST);
        //glEnable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        //glFrustum(-0.1, 0.1, -0.1, 0.1, 0.1, 100);
        //glOrtho(-100, 100, -100, 100, -100, 100);

        //glScalef(0.01, (0.01*xRes) / yRes, -0.01);
        glScalef(1.0/view_depth, 1.0/view_depth, -1.0/view_depth);

        //glTranslatef(0,0,-5);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // FIXME: Scene should be paused and resumed instead
        //realTicks = SDL_GetTicks();

        SDL_WM_GrabInput(SDL_GRAB_ON);
        SDL_ShowCursor(SDL_DISABLE);
        FlushMice();

        SDL_EnableKeyRepeat(0, 0);
        SDL_EnableUNICODE(0);
    }

    int DoFrame()
    {
        if (fatal_error)
            return 0;

        switch (appMode)
        {
        case MENU:
            {
                return MenuFrame();
            }
            break;
        case GAME:
            {
                return GameFrame();
            }
            break;
        }

        return 1;
    }

    inline void DrawString(void *font, const char string[])
    {
        int len = strlen(string);
        for (int i=0; i<len; i++)
            glutBitmapCharacter(font, (unsigned char)string[i]);
    }

    inline void DrawPlayer(Player &player)
    {
        glPushMatrix();
        glLoadIdentity();
        //glTranslatef(0,0,-0.5);

        glBegin(GL_LINE_STRIP);
        glColor3f(1,1,1);
        glVertex3d(0,0,0);



        //Quat4r arm1x(player.arm1.x*0.001, Vec3r(0,-1,0));
        //Quat4r arm1y(player.arm1.y*0.001, Vec3r(1,0,0));

        glColor3f(1,0,0);
        //glVertex3rv( &((arm1x*arm1y*arm1y*arm1x) * Vec3r(0,0,-1)).x );
        //glVertex3rv(&(player.arm1*0.002).x);
        {
            Vec3r v = Quat4r(player.arm1.Length()*0.002, Normalize(player.arm1.Cross(Vec3r(0,0,1)))) * Vec3r(0,0,-1);
            glVertex3rv( &v.x );
        }


        //Quat4r arm2x(player.arm2.x*0.001, Vec3r(0,-1,0));
        //Quat4r arm2y(player.arm2.y*0.001, Vec3r(1,0,0));

        glColor3f(0,0,1);
        //glVertex3rv( &((arm2x*arm2y*arm2y*arm2x) * Vec3r(0,0,-1)).x );
        //glVertex3rv(&(player.arm2*0.002).x);
        {
            Vec3r v = Quat4r(player.arm2.Length()*0.002, Normalize(player.arm2.Cross(Vec3r(0,0,1)))) * Vec3r(0,0,-1);
            glVertex3rv( &v.x );
        }

        glColor3f(1,1,1);
        glVertex3d(0,0,0);
        //glVertex3d(0,0,1);
        //glVertex3rv(&player.arm1.x);
        //glVertex3rv(&player.arm2.x);
        glEnd();
        glPopMatrix();
    }

    void DrawGame()
    {
        LOG_IF_ERROR("Start of DrawGame()");
        {
            std::lock_guard<std::mutex> lock(phyInstances_lock);
            switch (wireframemode)
            {
            case 1:
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glEnable(GL_CULL_FACE);
                break;
            case 2:
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDisable(GL_CULL_FACE);
                break;
            default:
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glEnable(GL_CULL_FACE);
            };

            shader.Enable();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glLoadIdentity();

            glRotatef( 90, -1, 0, 0);
            glRotatef( 90, 0, 0, 1);
            {
                Quat4r q = player.rot;
                q.Normalize();
                REAL a = acos( Clamp<REAL>(q.w, -1.0, 1.0) );
                Vec3r vec = Normalize(q.vec);
                glRotatef( a/M_PI*180.0*-2.0, vec.x, vec.y, vec.z );
            }

            GLfloat lightpos[4];
            lightpos[0] = 0; //player.pos.x;
            lightpos[1] = 0; //player.pos.y;
            lightpos[2] = 0; //player.pos.z;
            lightpos[3] = 1;
            glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

            //SDL_LockMutex(phyInstances_lock);
            if ( !phyInstances.empty() && !phyInstances.back().cameras.empty() )
            {
                Camera *cam = &phyInstances.back().cameras.begin()->second;

                static Vec3r last_delta(0, 0, 0);
                if (cam->rigid != NULL)
                {
                    Vec3r campos = cam->rigid->pos + (cam->rigid->orient) * cam->pos;
                    Vec3r delta = player.pos - campos;
                    if (delta.Length() >= 5)
                        player.pos = campos + Normalize(last_delta)*5;
                    else
                        last_delta = delta;

                    //player.pos = cam->rigid->pos + (cam->rigid->orient) * cam->pos;
                }
                else
                {
                    player.pos = cam->pos;
                    last_delta = player.pos;
                }

    /*
                player.pos += cam->pos2;

                Quat4r q = cam->rigid->orient * cam->orient;
                q.Normalize();
                REAL a = acos(Clamp<REAL>(q.w, -1.0, 1.0));
                Vec3r vec = Normalize(q.vec);
                glRotatef( a/M_PI*180.0*-2.0, vec.x, vec.y, vec.z );
    */
            }
            //SDL_UnlockMutex(phyInstances_lock);

            glTranslatef( -player.pos.x, -player.pos.y, -player.pos.z);

            glColor3f(1, 1, 1);

            if (stereo3dmode == 1) // cross-eyed
            {
                glViewport(0, 0, xRes/2, yRes);
                for (float a=0; a<2; ++a)
                {
                    if (world)
                        world->Draw();

                    //cgGLSetParameter4f( cg.param_PostAdjustments, (a==0? -stereo3d_depth*view_depth: stereo3d_depth*view_depth), (a==0? -stereo3d_focus/view_depth: stereo3d_focus/view_depth), 1.0, 2.0 / 3.0 );
                    DrawPlayer(player);
                    //SDL_LockMutex(phyInstances_lock);
                    typeof(phyInstances.begin()) it = phyInstances.begin(), end = phyInstances.end();
                    for (; it != end; ++it)
                        DrawPhysics(*it);
                    //SDL_UnlockMutex(phyInstances_lock);

                    glViewport(xRes/2, 0, xRes/2, yRes);
                }
                glViewport(0, 0, xRes, yRes);
            }
            else if (stereo3dmode == 2) // horizontally interlaced
            {
    #define X 0xff
                GLubyte stripple[]={X,X,X,X, 0,0,0,0, X,X,X,X, 0,0,0,0,
                                    X,X,X,X, 0,0,0,0, X,X,X,X, 0,0,0,0,
                                    X,X,X,X, 0,0,0,0, X,X,X,X, 0,0,0,0,
                                    X,X,X,X, 0,0,0,0, X,X,X,X, 0,0,0,0,
                                    X,X,X,X, 0,0,0,0, X,X,X,X, 0,0,0,0,
                                    X,X,X,X, 0,0,0,0, X,X,X,X, 0,0,0,0,
                                    X,X,X,X, 0,0,0,0, X,X,X,X, 0,0,0,0,
                                    X,X,X,X, 0,0,0,0, X,X,X,X, 0,0,0,0,
                                    X,X,X,X};
    #undef X
                glEnable(GL_POLYGON_STIPPLE);
                glPolygonStipple( stripple );
                for (float a=0; a<2; ++a)
                {
                    if (world)
                        world->Draw();

                    //cgGLSetParameter4f( cg.param_PostAdjustments, (a==0? -stereo3d_depth*view_depth: stereo3d_depth*view_depth), (a==0? -stereo3d_focus/view_depth: stereo3d_focus/view_depth), 1.0, 4.0 / 3.0 );
                    DrawPlayer(player);
                    //SDL_LockMutex(phyInstances_lock);
                    typeof(phyInstances.begin()) it = phyInstances.begin(), end = phyInstances.end();
                    for (; it != end; ++it)
                        DrawPhysics(*it);
                    //SDL_UnlockMutex(phyInstances_lock);

                    glPolygonStipple( stripple+4 );
                }
                glDisable(GL_POLYGON_STIPPLE);
            }
            else if (stereo3dmode == 3) // vertically interlaced
            {
    #define X 0x55
                GLubyte stripple[]={X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
    #undef X
    #define X 0xaa
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X,
                                    X,X,X,X, X,X,X,X, X,X,X,X, X,X,X,X};
    #undef X
                glEnable(GL_POLYGON_STIPPLE);
                glPolygonStipple( stripple );
                for (float a=0; a<2; ++a)
                {
                    if (world)
                        world->Draw();

                    //cgGLSetParameter4f( cg.param_PostAdjustments, (a==0? -stereo3d_depth*view_depth: stereo3d_depth*view_depth), (a==0? -stereo3d_focus/view_depth: stereo3d_focus/view_depth), 1.0, 4.0 / 3.0 );
                    DrawPlayer(player);
                    //SDL_LockMutex(phyInstances_lock);
                    typeof(phyInstances.begin()) it = phyInstances.begin(), end = phyInstances.end();
                    for (; it != end; ++it)
                        DrawPhysics(*it);
                    //SDL_UnlockMutex(phyInstances_lock);

                    glPolygonStipple( stripple+128 );
                }
                glDisable(GL_POLYGON_STIPPLE);
            }
            else // no stereo3d
            {
                if (world)
                    world->Draw();

                DrawPlayer(player);
                //SDL_LockMutex(phyInstances_lock);
                typeof(phyInstances.begin()) it = phyInstances.begin(), end = phyInstances.end();
                for (; it != end; ++it)
                    DrawPhysics(*it);
                //SDL_UnlockMutex(phyInstances_lock);
            }

            shader.Disable();
            glDisable(GL_LIGHT0);
        }

        profiler.RememberTime("DrawGame()");
        DrawHUD();

        SDL_GL_SwapBuffers();
        App::FlushConsole();
    }

    void DrawPhysics(PhyInstance &inst)
    {
        Physics *phys = inst.phys;

        if (!phys)
            return;

        inst.RecalculateNormals();
        LOG_IF_ERROR("RecalculateNormals");

        Vec3r **triangle_verts = inst.gltriangle_verts;
        Vec3r **triangle_normals = inst.gltriangle_normals;
        Vec3r **quad_verts = inst.glquad_verts;
        Vec3r **quad_normals =inst.glquad_normals;
        float *triangle_texcoords = inst.gltexcoords;
        float *quad_texcoords = inst.gltexcoords + inst.gltriangle_verts_count*2;

        for (unsigned int i = 0; i < inst.renderpasses_count; ++i)
        {
            Vec3r **triangle_verts_end = triangle_verts + inst.renderpasses[i].triangles_count*3;
            Vec3r **quad_verts_end = quad_verts + inst.renderpasses[i].quads_count*4;

            if (inst.renderpasses[i].colormap != NULL && inst.renderpasses[i].decalmap != NULL)
            {
                // Instuct opengl to mix texture and decal layer
                glActiveTextureARB(GL_TEXTURE0_ARB);
                inst.renderpasses[i].colormap->Enable();
                inst.renderpasses[i].colormap->Bind();

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

                // Instruct opengl to blend the combined texture with the lighting calculations
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glEnable(GL_TEXTURE_2D);
                inst.renderpasses[i].decalmap->Bind();

                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_COMBINE);
                glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB, GL_MODULATE);
                glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
                glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB, GL_PREVIOUS);
                glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB, GL_SRC_COLOR);
                glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB, GL_SRC_COLOR);
                LOG_IF_ERROR("glTexEnvi#2")
            }
            else if (inst.renderpasses[i].colormap != NULL)
            {
                inst.renderpasses[i].colormap->Enable();
                inst.renderpasses[i].colormap->Bind();
            }

            if (inst.renderpasses[i].envmap != NULL)
            {
                glActiveTextureARB(GL_TEXTURE2_ARB);
                inst.renderpasses[i].envmap->Enable();
                inst.renderpasses[i].envmap->Bind();
                //*
                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_COMBINE);
                glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB, GL_ADD_SIGNED);
                glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB, GL_PREVIOUS);
                glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB, GL_TEXTURE2);
                glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB, GL_SRC_COLOR);
                glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB, GL_SRC_COLOR);
                /*/
                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_COMBINE);
                glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB, GL_INTERPOLATE);
                glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB, GL_PREVIOUS);
                glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB, GL_TEXTURE2);
                glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE2_RGB, GL_TEXTURE1);
                glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB, GL_SRC_COLOR);
                glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB, GL_SRC_COLOR);
                glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND2_RGB, GL_SRC_ALPHA);
                //*/

                LOG_IF_ERROR("glTexEnvi#3")
                glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_ALPHA, GL_REPLACE);
                glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_ALPHA, GL_PREVIOUS);
                glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
                LOG_IF_ERROR("glTexEnvi#4")
            }

            glBegin(GL_TRIANGLES);
            while (triangle_verts < triangle_verts_end)
            {
                glMultiTexCoord2fARB(GL_TEXTURE0_ARB, triangle_texcoords[0], triangle_texcoords[1]);
                glMultiTexCoord2fARB(GL_TEXTURE1_ARB, triangle_texcoords[0], triangle_texcoords[1]);
                glNormal3rv(&(*triangle_normals)->x);
                glVertex3rv(&(*triangle_verts)->x);

                triangle_verts += 1;
                triangle_normals += 1;
                triangle_texcoords += 2;
            }
            glEnd();

            glBegin(GL_QUADS);
            while (quad_verts < quad_verts_end)
            {
                glMultiTexCoord2fARB(GL_TEXTURE0_ARB, quad_texcoords[0], quad_texcoords[1]);
                glMultiTexCoord2fARB(GL_TEXTURE1_ARB, quad_texcoords[0], quad_texcoords[1]);
                glNormal3rv(&(*quad_normals)->x);
                glVertex3rv(&(*quad_verts)->x);

                quad_verts += 1;
                quad_normals += 1;
                quad_texcoords += 2;
            }
            glEnd();

            /*
            (1-a)*(colormap*col + envmap) + a*decalmap*col
            colormap*col
            +envmap
            interpolate(prev, decalmap, a)

            ((1-a)*colormap + a*decalmap)*col + (1-a)*envmap

            a*((1-a)*colormap + a*decalmap)*col + (1-a)*envmap


            */

            if (inst.renderpasses[i].envmap != NULL)
            {
                glActiveTextureARB(GL_TEXTURE2_ARB);
                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_MODULATE);
                inst.renderpasses[i].envmap->Disable();
            }

            if (inst.renderpasses[i].decalmap != NULL)
            {
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_MODULATE);
                inst.renderpasses[i].decalmap->Disable();
            }

            if (inst.renderpasses[i].colormap != NULL)
            {
                glActiveTextureARB(GL_TEXTURE0_ARB);
                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_MODULATE);
                inst.renderpasses[i].colormap->Disable();
            }
            glActiveTextureARB(GL_TEXTURE0_ARB);

        }

        glNormal3f(0, 0, 1);

        // Draw bounds
        if (App::developermode)
        {
            //  normal length should be 1: a*a + a*a + a*a = 1*1
            float a = sqrt(1.0/3.0);
            float bounds[] =
            {
                phys->bounds_min.x, phys->bounds_min.y, phys->bounds_min.z, -a,-a,-a,
                phys->bounds_min.x, phys->bounds_min.y, phys->bounds_max.z, -a,-a, a,
                phys->bounds_min.x, phys->bounds_max.y, phys->bounds_min.z, -a, a,-a,
                phys->bounds_min.x, phys->bounds_max.y, phys->bounds_max.z, -a, a, a,

                phys->bounds_max.x, phys->bounds_min.y, phys->bounds_min.z,  a,-a,-a,
                phys->bounds_max.x, phys->bounds_min.y, phys->bounds_max.z,  a,-a, a,
                phys->bounds_max.x, phys->bounds_max.y, phys->bounds_min.z,  a, a,-a,
                phys->bounds_max.x, phys->bounds_max.y, phys->bounds_max.z,  a, a, a
            };

            unsigned int indices[] =
            {
                0,1,3,2,
                4,5,7,6,
                0,4,1,5,3,7,2,6
            };
            GLboolean lighting = false;
            glGetBooleanv(GL_LIGHTING, &lighting);

            glDisable(GL_LIGHTING);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);

            glVertexPointer(3, GL_FLOAT, sizeof(float)*6, static_cast<void*>(bounds));
            glNormalPointer(GL_FLOAT, sizeof(float)*6, static_cast<void*>(bounds+3));

            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, static_cast<void*>(indices));
            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, static_cast<void*>(indices+4));
            glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, static_cast<void*>(indices+8));

            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);

            if (lighting)
                glEnable(GL_LIGHTING);
        }
    }

    void DrawHUD()
    {
        if (!profilermode)
            return;

        glDisable(GL_DEPTH_TEST);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, xRes, 0, yRes);

        glColor3f(0.8, 0.8, 0.8);
        int height = 15; //glutBitmapHeight(GLUT_BITMAP_HELVETICA_18);
        int y = yRes;

#define times (profiler.times)
#define names (profiler.names)
#define freq (profiler.freq)
        if ( times.size() > 0 )
        {
            double inv_freq = 1.0e-9;
            typeof(times.back()) total = 0;

            for ( typeof(times.begin()) it = times.begin(); it != times.end(); ++it )
                total += *it;

            {
                std::stringstream ss;
                ss << "FPS: " << std::fixed << floor(1000.0 / (total * inv_freq) + 0.5);
                glRasterPos2f( 0, y-=height );
                DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
            }

            for ( unsigned int j = 0; j < names.size(); ++j )
            {
                std::stringstream ss;
                double time = times[j];
                ss.width(20);
                ss << std::left << names[j] + ":";
                ss.width(10);
                ss << std::fixed << std::right << time / total * 100.0 << "% ";
                ss.width(10);
                ss << std::fixed << std::right << time * inv_freq << " ms";
                glRasterPos2f( 0, y-=height );
                DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
            }

            std::stringstream ss;
            ss << "total: " << std::fixed << total * inv_freq << " ms";
            glRasterPos2f( 0, y-=height );
            DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
        }
#undef times
#undef names
#undef freq

        {
            std::lock_guard<std::mutex> lock(phyInstances_lock);
            typeof(phyInstances.begin()) it;
            for (it = phyInstances.begin(); it != phyInstances.end() && y > 0; ++it)
            {
#define times (it->phys->profiler.times)
#define names (it->phys->profiler.names)
#define freq (it->phys->profiler.freq)
                if ( times.size() > 0 )
                {
                    double inv_freq = 1.0e-9;
                    typeof(times.back()) total = 0;

                    for ( typeof(times.begin()) it2 = times.begin(); it2 != times.end(); ++it2 )
                        total += *it2;

                    for ( unsigned int j = 0; j < names.size(); ++j )
                    {
                        std::stringstream ss;
                        double time = times[j];
                        ss.width(20);
                        ss << std::left << names[j] + ":";
                        ss.width(10);
                        ss << std::fixed << std::right << time / total * 100.0 << "% ";
                        ss.width(10);
                        ss << std::fixed << std::right << time * inv_freq << " ms";
                        glRasterPos2f( 0, y-=height );
                        DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
                    }

                    std::stringstream ss;
                    ss << "total: " << std::fixed << total * inv_freq << " ms";
                    glRasterPos2f( 0, y-=height );
                    DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
                }
#undef times
#undef names
#undef freq
            }
        }

/*
        {
            std::stringstream ss;
            ss << "zrot: " << player.zrot;
            glRasterPos2f( 0, y-=height );
            DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
        }
        {
            std::stringstream ss;
            ss << "xrot: " << player.xrot;
            glRasterPos2f( 0, y-=height );
            DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
        }
        {
            std::stringstream ss;
            ss << "zrot2: " << player.zrot2;
            glRasterPos2f( 0, y-=height );
            DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
        }
        {
            std::stringstream ss;
            ss << "xrot2: " << player.xrot2;
            glRasterPos2f( 0, y-=height );
            DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
        }
*/
        {
            std::stringstream ss;
            ss << "device: " << last_active_device;
            glRasterPos2f( 0, y-=height );
            DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
        }
        {
            std::stringstream ss;
            ss << "rot: " << std::fixed << std::showpos << Euler(player.rot) * (180.0/M_PI);
            glRasterPos2f( 0, y-=height );
            DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
        }
        {
            std::stringstream ss;
            ss << "arm1: " << std::fixed << std::showpos << player.arm1;
            glRasterPos2f( 0, y-=height );
            DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
        }
        {
            std::stringstream ss;
            ss << "arm2: " << std::fixed << std::showpos << player.arm2;
            glRasterPos2f( 0, y-=height );
            DrawString( GLUT_BITMAP_9_BY_15, ss.str().c_str() );
        }
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
    }

    int GameFrame()
    {
        if ( SCENE_DRAW & scene->GetFlags() )
        {
            DrawGame();
            scene->RemoveFlags(SCENE_DRAW);
            ParseEvent("frame", 1, 1);
        }

        profiler.Reset();

        {
            int retval = HandleGameEvents();
            if (retval != -1)
                return retval;
        }

        SDL_Delay(1);

        return 1;
    }

    void ReloadVideo()
    {
        SDL_SetVideoMode(xRes, yRes, 0,
                         (fullscreen ? SDL_FULLSCREEN : 0) | SDL_OPENGL /*| SDL_HWSURFACE | SDL_NOFRAME*/);

        Texture::ReloadAll();
        Shader::ReloadAll();
        if (world)
            world->BuildSky();

        SwitchMode(appMode);
    }

    void SwitchMode(AppMode mode)
    {
        // if (appMode == mode)
        //  return;
        appMode = mode;

        switch (appMode)
        {
        case MENU:
            SetupMenu();
            break;
        case GAME:
            SetupGame();
            break;
        }
    }

    void LoadWorld(const char filename[])
    {
        PhyInstance::DeleteAll();
        delete world;
        world = new World(filename);
        LOG_S(INFO) << "Loaded world from \"" << filename << "\"";
    }

    void SaveWorld(const char filename[])
    {
        world->SaveTo(filename);
        LOG_S(INFO) << "Saved world to \"" << filename << "\"";
    }

    void ClearWorld()
    {
        PhyInstance::DeleteAll();
        delete world;
        world = NULL;
        //world->Clear();
    }

    void SaveScreenshot(const char filename[])
    {
        SDL_Surface *temp = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, screen->h, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            0x000000FF, 0x0000FF00, 0x00FF0000, 0
#else
            0x00FF0000, 0x0000FF00, 0x000000FF, 0
#endif
        );
        if (temp == NULL)
        {
            LOG_S(ERROR) << "Failed to save screenshot: Could not create temporary surface\"";
            return;
        }

        glReadPixels(0, 0, screen->w, screen->h, GL_RGB, GL_UNSIGNED_BYTE, temp->pixels);
        FlipImageY(temp);

        // TODO: Save as png instead and use physfs
        if ( 0 == SDL_SaveBMP(temp, filename) )
        {
            LOG_S(INFO) << "Saved screenshot to \"" << filename << "\"";
        }
        else
        {
            LOG_S(ERROR) << "Failed to save screenshot: " << SDL_GetError();
        }

        SDL_FreeSurface(temp);
    }

} // namespace App
