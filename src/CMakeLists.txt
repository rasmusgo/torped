file(GLOB sources *.cpp *.hpp *.c *.h)

add_executable(torped ${sources})

target_precompile_headers(torped PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp>"
)

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
    Eigen3::Eigen
)

if(WIN32)
    target_link_libraries(torped PRIVATE legacy_stdio_definitions.lib)
else()
    target_compile_options(torped PRIVATE -Wno-deprecated)
endif()
