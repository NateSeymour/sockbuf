cmake_minimum_required(VERSION 3.21)
project(sockbuf VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

# LIBS
## GTEST
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
option(INSTALL_GTEST "Enable installation of googletest." OFF)
FetchContent_MakeAvailable(googletest)

# sockbuf

add_library(sockbuf src/sockbuf.cpp src/sockbuf.h)

add_executable(sockbuf-test test/sockbuf.test.cpp)
target_link_libraries(sockbuf-test PUBLIC sockbuf gtest_main)