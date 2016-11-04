file(GLOB sources manymouse/*.c)

add_library(manymouse ${sources})

target_link_libraries(manymouse LINK_PRIVATE
    dl
)

add_subdirectory(angelscript/sdk/angelscript/projects/cmake)
