#pragma once
#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>

#define LOG_IF_ERROR(msg) \
{ \
    GLenum err; \
    while ( (err = glGetError()) != GL_NO_ERROR ) \
    { \
        switch (err) \
        { \
            case GL_NO_ERROR:          LOG_S(ERROR) << msg << ": GL_NO_ERROR";          break; \
            case GL_INVALID_ENUM:      LOG_S(ERROR) << msg << ": GL_INVALID_ENUM";      break; \
            case GL_INVALID_VALUE:     LOG_S(ERROR) << msg << ": GL_INVALID_VALUE";     break; \
            case GL_INVALID_OPERATION: LOG_S(ERROR) << msg << ": GL_INVALID_OPERATION"; break; \
            case GL_OUT_OF_MEMORY:     LOG_S(ERROR) << msg << ": GL_OUT_OF_MEMORY";     break; \
            default:                   ABORT_S()    << msg << ": Unknown GL ERROR: " << (int)err; \
        } \
    } \
}
