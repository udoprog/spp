set(spp_SOURCES
  subprocess.cpp
  session.cpp
)

add_executable(spp main.cpp ${spp_SOURCES})
add_executable(spp-client client.cpp)
add_executable(spp-debug main.cpp ${spp_SOURCES})

set_target_properties(spp PROPERTIES COMPILE_FLAGS "-O3 -Wall")
set_target_properties(spp-debug PROPERTIES COMPILE_FLAGS "-Wall -pedantic -g")

target_link_libraries(spp ${spp_LIBRARIES})
target_link_libraries(spp-debug ${spp_LIBRARIES})
target_link_libraries(spp-client ${spp_LIBRARIES})
