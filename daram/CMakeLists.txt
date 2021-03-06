cmake_minimum_required(VERSION 3.12)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

project(Daram CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(DARAM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(CSDAutomata INTERFACE)
target_include_directories(CSDAutomata INTERFACE ${DARAM_INCLUDE_DIR})

if(NOT TARGET sim_ds)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/SimpleDataStructure EXCLUDE_FROM_ALL)
endif()
target_link_libraries(CSDAutomata INTERFACE sim_ds)

message(STATUS "Make Daram")
message(STATUS "BUILD_TYPE          is  ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_FLAGS           are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG     are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE   are ${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory(tools)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    enable_testing()
    add_subdirectory(test)
endif()

