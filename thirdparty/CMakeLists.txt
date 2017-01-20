file(GLOB sources manymouse/*.c)

add_library(manymouse ${sources})

target_link_libraries(manymouse LINK_PRIVATE
    dl
)

if(APPLE)
   find_library(CARBON_LIBRARY Carbon)
   find_library(IOKIT_LIBRARY IOKit )
   mark_as_advanced(CARBON_LIBRARY IOKIT_LIBRARY)
   target_link_libraries(manymouse LINK_PRIVATE
       ${CARBON_LIBRARY} ${IOKIT_LIBRARY}
   )
endif(APPLE)

add_subdirectory(angelscript/sdk/angelscript/projects/cmake)
