file(GLOB MDRPC_SOURCES *.cpp *.hpp)

set(CMAKE_CXX_STANDARD 17)

add_library(mdrpc SHARED ${MDRPC_SOURCES})
target_link_libraries(mdrpc discord-rpc)
