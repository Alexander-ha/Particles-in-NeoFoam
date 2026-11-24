add_library(particles INTERFACE)
target_include_directories(particles INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(particles INTERFACE Kokkos::kokkos)