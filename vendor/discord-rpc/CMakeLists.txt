# Modified from upstream discord-rpc 
# to make it easier to build as a submodule.
option(ENABLE_IO_THREAD "Start up a separate I/O thread, otherwise I'd need to call an update function" ON)
option(WARNINGS_AS_ERRORS "When enabled, compiles with `-Werror` (on *nix platforms)." OFF)

set(CMAKE_CXX_STANDARD 14)

set(BASE_RPC_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/discord_rpc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/discord_register.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/discord_rpc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpc_connection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpc_connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/serialization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/serialization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backoff.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/msg_queue.h
)

if(WIN32)
    add_definitions(-DDISCORD_WINDOWS)
    set(BASE_RPC_SRC ${BASE_RPC_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/src/connection_win.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/discord_register_win.cpp)
    add_library(discord-rpc ${BASE_RPC_SRC})
    if (MSVC)
        target_compile_options(discord-rpc PRIVATE /EHsc
            /Wall
            /wd4100 # unreferenced formal parameter
            /wd4514 # unreferenced inline
            /wd4625 # copy constructor deleted
            /wd5026 # move constructor deleted
            /wd4626 # move assignment operator deleted
            /wd4668 # not defined preprocessor macro
            /wd4710 # function not inlined
            /wd4711 # function was inlined
            /wd4820 # structure padding
            /wd4946 # reinterpret_cast used between related classes
            /wd5027 # move assignment operator was implicitly defined as deleted
        )
    endif(MSVC)
    target_link_libraries(discord-rpc PRIVATE psapi advapi32)
endif(WIN32)

if(UNIX)
    set(BASE_RPC_SRC ${BASE_RPC_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/src/connection_unix.cpp)

    if (APPLE)
        add_definitions(-DDISCORD_OSX)
        set(BASE_RPC_SRC ${BASE_RPC_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/src/discord_register_osx.m)
    else (APPLE)
        add_definitions(-DDISCORD_LINUX)
        set(BASE_RPC_SRC ${BASE_RPC_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/src/discord_register_linux.cpp)
    endif(APPLE)

    add_library(discord-rpc ${BASE_RPC_SRC})
    target_link_libraries(discord-rpc PUBLIC pthread)

    target_compile_options(discord-rpc PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -fPIC
    )

    if (${WARNINGS_AS_ERRORS})
      target_compile_options(discord-rpc PRIVATE -Werror)
    endif (${WARNINGS_AS_ERRORS})

    target_compile_options(discord-rpc PRIVATE
        -Wno-unknown-pragmas # pragma push thing doesn't work on clang
        -Wno-old-style-cast # it's fine
#        -Wno-c++98-compat # that was almost 2 decades ago
#       -Wno-c++98-compat-pedantic
        -Wno-missing-noreturn
        -Wno-padded # structure padding
        -Wno-covered-switch-default
        -Wno-exit-time-destructors # not sure about these
        -Wno-global-constructors
    )

    if (APPLE)
        target_link_libraries(discord-rpc PRIVATE "-framework AppKit")
    endif (APPLE)
endif(UNIX)

target_include_directories(discord-rpc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(discord-rpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include)

if (NOT ${ENABLE_IO_THREAD})
    target_compile_definitions(discord-rpc PUBLIC -DDISCORD_DISABLE_IO_THREAD)
endif (NOT ${ENABLE_IO_THREAD})
