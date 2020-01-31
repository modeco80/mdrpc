file(GLOB MDRPC_SOURCES *.cpp ../include/*.hpp)

add_library(mdrpc2 SHARED ${MDRPC_SOURCES})
target_link_libraries(mdrpc2 discord-rpc)
