file(GLOB sources *.cpp *.hpp)

add_executable(torped ${sources})

target_link_libraries(torped LINK_PRIVATE ${SDL_LIBRARY})

target_compile_features(torped PRIVATE cxx_range_for)
