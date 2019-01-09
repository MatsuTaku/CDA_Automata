cmake_minimum_required(VERSION 3.12)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

project(Daram)

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

set(DARAM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB HEADER_FILES ${DARAM_INCLUDE_DIR}/csd_automata.hpp ${DARAM_INCLUDE_DIR}/csd_automata/*.hpp)

# Build Interface Library
add_library(CSDAutomata STATIC ${DARAM_INCLUDE_DIR}/csd_automata.cpp ${HEADER_FILES})
target_include_directories(CSDAutomata PUBLIC ${DARAM_INCLUDE_DIR})

add_subdirectory(lib/poplar-trie)
set(POPLAR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/poplar-trie/include")
target_include_directories(CSDAutomata PUBLIC ${POPLAR_INCLUDE_DIR})

set(SIMDS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/SimpleDataStructure/include")
target_include_directories(CSDAutomata PUBLIC ${SIMDS_INCLUDE_DIR})

message(STATUS "Make Daram")
message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE are ${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory(tools)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    enable_testing()
    add_subdirectory(test)
endif()

