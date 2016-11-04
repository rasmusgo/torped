file(GLOB sources manymouse/*.c)

add_library(manymouse ${sources})

target_link_libraries(manymouse LINK_PRIVATE
    dl
)
