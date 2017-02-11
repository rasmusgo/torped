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
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_LIBRARY}
    ${TINYXML_LIBRARY}
    ${VORBIS_LIBRARY}
    ${VORBISFILE_LIBRARY}
    manymouse
)
target_compile_options(torped PRIVATE -Wno-deprecated)
