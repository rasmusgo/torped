file(GLOB sources *.cpp *.hpp)

add_executable(torped ${sources})

target_compile_features(torped PRIVATE cxx_range_for)

target_link_libraries(torped LINK_PRIVATE
    ${GLEW_LIBRARIES}
    ${GLUT_glut_LIBRARY}
    ${LUA_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${PHYSFS_LIBRARY}
    ${PYTHON_LIBRARIES}
    ${SDL_LIBRARY}
    tinyxml
)
