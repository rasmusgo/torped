#pragma once
#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>

#define LOG_IF_ERROR(msg) \
{ \
    GLenum err; \
    while ( (err = glGetError()) != GL_NO_ERROR ) \
    { \
        LOG_S(ERROR) << msg << ": GL ERROR: " << gluErrorString(err) << std::endl; \
    } \
}
