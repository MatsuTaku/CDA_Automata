cmake_minimum_required(VERSION 3.12)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

project(SDExperiments)

set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

include(CheckSSE4_2)
if(BUILTIN_POPCNT)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang|Intel")
        set(CMAKE_CXX_FLAGS "-msse4.2 -march=native ${CMAKE_CXX_FLAGS}")
    endif()
endif()

enable_testing()

set(Daram_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/../daram/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../daram/lib/SimpleDataStructure/include"
)

set(SUCCINCT_USE_POPCNT ON)
set(SUCCINCT_USE_INTRINSICS ON)
add_subdirectory(lib/path_decomposed_tries/succinct EXCLUDE_FROM_ALL)

find_package(Boost 1.42.0 COMPONENTS 
	unit_test_framework iostreams system thread filesystem REQUIRED)

include_directories(include ${Daram_INCLUDE_DIRS} /usr/local/include lib/path_decomposed_tries ${Boost_INCLUDE_DIRS})
link_directories(/usr/local/lib ${Boost_LIBRARY_DIRS})

message(STATUS "Make SDExperiments")
message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE are ${CMAKE_CXX_FLAGS_RELEASE}")

file(GLOB WRAPPERS include/*Wrapper.hpp)
add_executable(sd_experiments src/experiment.cpp ${WRAPPERS})
target_link_libraries(sd_experiments xcdat marisa succinct ${Boost_LIBRARIES})

add_executable(darts_build src/darts_build.cpp include/DartsCloneWrapper.hpp)

