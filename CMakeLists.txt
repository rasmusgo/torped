cmake_minimum_required(VERSION 2.8.11)
project(TORPED_PROJECT)

find_package(GLEW REQUIRED)

find_package(GLUT REQUIRED)

find_package(OpenAL REQUIRED)

find_package(OpenGL REQUIRED)

find_package(PhysFS REQUIRED)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

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
)

add_subdirectory(src)
