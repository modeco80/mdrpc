file(GLOB MDRPC_SOURCES *.cpp *.hpp)

set(CMAKE_CXX_STANDARD 17)

add_library(mdrpc2 SHARED ${MDRPC_SOURCES})
target_link_libraries(mdrpc2 discord-rpc)
