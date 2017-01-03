# sudo apt install        \
#     freeglut3-dev       \
#     libglew-dev         \
#     libglut-dev         \
#     liblua5.1-dev       \
#     libopenal-dev       \
#     libphysfs-dev       \
#     libsdl-image1.2-dev \
#     libsdl-net1.2-dev   \
#     libsdl-ttf2.0-dev   \
#     libtinyxml-dev      \
#     libvorbis-dev       \
#     python2.7-dev

cmake_minimum_required(VERSION 2.8.7)
project(TORPED_PROJECT)

find_package(GLEW REQUIRED)

find_package(GLUT REQUIRED)

find_package(OpenAL REQUIRED)

find_package(OpenGL REQUIRED)

find_package(PhysFS REQUIRED)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

find_package(SDL_image REQUIRED)

find_package(Lua51 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

find_package(PythonLibs 2.7 REQUIRED)
if (PYTHONLIBS_FOUND)
    include_directories(${PYTHON_INCLUDE_DIRS})
endif ()

add_subdirectory(thirdparty)

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/angelscript/sdk/angelscript/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/manymouse"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/loguru"
)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_subdirectory(src)
