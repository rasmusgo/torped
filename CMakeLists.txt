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
#     libvorbis-dev

# sudo port install git-lfs
# git lfs install
# git lfs fetch
# git lfs checkout
# sudo port install glew physfs libsdl2 libsdl2_image lua51 tinyxml libvorbis libogg
# brew install cmake git-lfs glew physfs sdl2 sdl2_image lua51 tinyxml libvorbis libogg

cmake_minimum_required(VERSION 3.0.2)
project(TORPED_PROJECT)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})

find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(PhysFS REQUIRED)
include_directories(${PHYSFS_INCLUDE_DIR})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIR})

find_package(Lua51 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

find_library(OGG_LIBRARY ogg)
find_library(VORBIS_LIBRARY vorbis)
find_library(VORBISFILE_LIBRARY vorbisfile)
find_library(TINYXML_LIBRARY tinyxml)

add_subdirectory(thirdparty)

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/manymouse"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/loguru"
)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_subdirectory(src)
