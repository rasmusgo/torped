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

# Ogg from https://github.com/xiph/ogg/releases
# Vorbis from https://github.com/xiph/vorbis/releases

cmake_minimum_required(VERSION 3.0.2)
project(TORPED_PROJECT)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/thirdparty/glew-2.1.0")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/thirdparty/freeglut")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/thirdparty/physfs-3.0.2/installdir")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/thirdparty/SDL2-2.0.12")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/thirdparty/SDL2_image-2.0.5")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/thirdparty/installdir")
list(APPEND CMAKE_PREFIX_PATH "C:/Program Files (x86)/OpenAL 1.1 SDK")

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})
if(NOT DEFINED GLEW_LIBRARIES)
    set(GLEW_LIBRARIES GLEW::GLEW)
endif()

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
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/installdir/include"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_subdirectory(src)
