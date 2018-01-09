cmake_minimum_required(VERSION 2.8)
project(array_fsa)

set(CMAKE_CXX_FLAGS "-Wall -std=c++1z")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE are ${CMAKE_CXX_FLAGS_RELEASE}")

set(HEADER_FILES
  ArrayFSA.hpp
  NArrayFSA.hpp
  NArrayFSADACs.hpp
  NArrayFSATextEdge.hpp
  ArrayFSABuilder.hpp
  ArrayFSATailBuilder.hpp
  ArrayTri.hpp
  Rank.hpp
  basic.hpp
  Exception.hpp
  FsaTools.hpp
  FsaGenerator.hpp
  MorfologikCFSA2.hpp
  MorfologikFSA5.hpp
  PlainFSA.hpp
  PlainFSABuilder.hpp
  StringDict.hpp
  StringDictBuilder.hpp
  StrDictData.hpp
  DACs.hpp
  DacUnit.hpp
  FitValuesArray.hpp
  ByteData.hpp
  Calc.hpp
)

set(SOURCE_FILES
  ArrayFSABuilder.cpp
  ArrayFSATailBuilder.cpp
  PlainFSABuilder.cpp
  StringDictBuilder.cpp
)

add_library(ArrayFSA STATIC
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

add_executable(test_PlainFSA test_PlainFSA.cpp)
target_link_libraries(test_PlainFSA ArrayFSA)

add_executable(bench bench.cpp)
target_link_libraries(bench ArrayFSA)

add_executable(build build.cpp)
target_link_libraries(build ArrayFSA)

