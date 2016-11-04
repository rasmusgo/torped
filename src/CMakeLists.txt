file(GLOB sources *.cpp *.hpp)

add_executable(torped ${sources})

target_compile_features(torped PRIVATE cxx_range_for)

target_link_libraries(torped LINK_PRIVATE
    ${GLEW_LIBRARIES}
    ${GLUT_glut_LIBRARY}
    ${OPENAL_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${PYTHON_LIBRARIES}
    ${SDL_LIBRARY}
    tinyxml
)
