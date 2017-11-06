cmake_minimum_required(VERSION 2.8)
project(array_fsa)

set(CMAKE_CXX_FLAGS "-Wall -std=c++14")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE are ${CMAKE_CXX_FLAGS_RELEASE}")

set(HEADER_FILES
  ArrayFSA.hpp
  ArrayFSABuilder.hpp
  ArrayFSATailBuilder.hpp
  ArrayDACFSA.hpp
  ArrayDACFSABuilder.hpp
  ArrayDACFSARank.hpp
  Rank.hpp
  basic.hpp
  FsaTools.hpp
  MorfologikCFSA2.hpp
  MorfologikFSA5.hpp
  PlainFSA.hpp
  PlainFSABuilder.hpp
  )

set(SOURCE_FILES
  ArrayFSABuilder.cpp
  ArrayFSATailBuilder.cpp
  ArrayDACFSABuilder.cpp
  PlainFSABuilder.cpp
)

add_executable(test_PlainFSA test_PlainFSA.cpp ${SOURCE_FILES} ${HEADER_FILES})
add_executable(bench bench.cpp ${SOURCE_FILES} ${HEADER_FILES})
add_executable(build build.cpp ${SOURCE_FILES} ${HEADER_FILES})
