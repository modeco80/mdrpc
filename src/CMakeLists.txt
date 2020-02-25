file(GLOB MDRPC_SOURCES *.cpp *.hpp)

set(CMAKE_CXX_STANDARD 17)

add_custom_target(vergen COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/vergen.cmake WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Version.hpp)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/Version.hpp PROPERTIES GENERATED TRUE)

add_library(mdrpc SHARED ${MDRPC_SOURCES})
add_dependencies(mdrpc vergen)
target_include_directories(mdrpc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(mdrpc discord-rpc)
