set(spp_SOURCES main.cpp)

add_executable(spp ${spp_SOURCES})
add_executable(spp-debug ${spp_SOURCES})

set_target_properties(spp PROPERTIES COMPILE_FLAGS "-O3 -Wall")
set_target_properties(spp-debug PROPERTIES COMPILE_FLAGS "-Wall -pedantic -g")
