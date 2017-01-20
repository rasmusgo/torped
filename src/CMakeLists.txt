file(GLOB sources *.cpp *.hpp *.c *.h)

add_executable(torped ${sources})

target_link_libraries(torped LINK_PRIVATE
    ${GLEW_LIBRARIES}
    ${GLUT_glut_LIBRARY}
    ${LUA_LIBRARIES}
    ${OGG_LIBRARY}
    ${OPENAL_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${PHYSFS_LIBRARY}
    ${SDL_IMAGE_LIBRARIES}
    ${SDL_LIBRARY}
    ${TINYXML_LIBRARY}
    ${VORBIS_LIBRARY}
    ${VORBISFILE_LIBRARY}
    angelscript
    manymouse
    pthread
)
