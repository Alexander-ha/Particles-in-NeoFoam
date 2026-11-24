add_library(coupling INTERFACE)
target_include_directories(coupling INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(coupling INTERFACE particles NeoN::NeoN)