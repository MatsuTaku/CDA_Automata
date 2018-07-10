cmake_minimum_required(VERSION 3.1)

project(array_fsa)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "-Wall -std=c++1z")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE are ${CMAKE_CXX_FLAGS_RELEASE}")

set(IncludeDir ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB HEADER_FILES ${IncludeDir}/array_fsa.hpp ${IncludeDir}/array_fsa/*.hpp)
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(ArrayFSA STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(ArrayFSA PUBLIC ${IncludeDir})

add_subdirectory(lib/SimDS)
target_link_libraries(ArrayFSA PUBLIC sim_ds)


add_executable(test_PlainFSA test_PlainFSA.cpp)
target_link_libraries(test_PlainFSA ArrayFSA)

add_executable(bench bench.cpp FsaTools.hpp)
target_link_libraries(bench ArrayFSA)

add_executable(build build.cpp FsaGenerator.hpp)
target_link_libraries(build ArrayFSA)

add_executable(makefsa makefsa.cpp FsaGenerator.hpp)
target_link_libraries(makefsa ArrayFSA)

add_executable(build_array build_array.cpp)
target_link_libraries(build_array ArrayFSA)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	message(STATUS "Enable test ArrayFSA")
	enable_testing()
	add_subdirectory(test)
endif()

