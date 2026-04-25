cmake_minimum_required(VERSION 3.14)
project(transport_navigator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(navigator_core
    src/domain/graph.cpp
    src/domain/time_variants.cpp
    src/application/strategies.cpp
    src/application/a_star_solver.cpp
    src/infrastructure/csv_parser.cpp
    src/infrastructure/graph_builder.cpp
)

target_include_directories(navigator_core PUBLIC include)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(navigator_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(navigator_tests
    tests/test_graph.cpp
    tests/test_static_logic.cpp
    tests/test_a_star.cpp
    tests/test_csv_parser.cpp
    tests/test_graph_builder.cpp
)
target_link_libraries(navigator_tests PRIVATE navigator_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(navigator_tests)
