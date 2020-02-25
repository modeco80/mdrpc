#!/usr/bin/cmake -P
set(VERSION_FILENAME "Version.hpp")
execute_process(COMMAND git describe --tags --always HEAD OUTPUT_VARIABLE GIT_TAG OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE)

file(WRITE ${VERSION_FILENAME} "// Do not edit\n")
file(APPEND ${VERSION_FILENAME} "#pragma once\n")
file(APPEND ${VERSION_FILENAME} "namespace mdrpc {\n") 
file(APPEND ${VERSION_FILENAME} "namespace Version {\n\n")
file(APPEND ${VERSION_FILENAME} "constexpr static char tag[] = \"${GIT_TAG}\";\n")
file(APPEND ${VERSION_FILENAME} "constexpr static char branch[] = \"${GIT_BRANCH}\";\n")
file(APPEND ${VERSION_FILENAME} "\n}")
file(APPEND ${VERSION_FILENAME} "\n}")
message(STATUS "Generated ${VERSION_FILENAME} for ${GIT_TAG}-${GIT_BRANCH}.")
