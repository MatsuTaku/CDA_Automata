cmake_minimum_required(VERSION 3.12)

project(sd_experiments)

set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE are ${CMAKE_CXX_FLAGS_RELEASE}")

enable_testing()

link_directories(/usr/local/lib)
include_directories(/usr/local/include include lib/path_decomposed_tries)

find_package(Boost 1.42.0 COMPONENTS unit_test_framework iostreams system thread filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_subdirectory(project)

add_subdirectory(lib/path_decomposed_tries/succinct EXCLUDE_FROM_ALL)

file(GLOB WRAPPERS include/*Wrapper.hpp)
add_executable(sd_experiments src/experiment.cpp ${WRAPPERS})
target_link_libraries(sd_experiments
    CSDAutomata xcdat marisa succinct ${Boost_LIBRARIES})

add_executable(darts_build src/darts_build.cpp include/DartsCloneWrapper.hpp)
