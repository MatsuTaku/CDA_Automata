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
  ArrayFSABuilder.hpp
  ArrayFSATailBuilder.hpp
  ArrayTri.hpp
  BitVector.hpp
  basic.hpp
  Exception.hpp
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
  NextCheck.hpp
  FSA.hpp
  StringTransFSA.hpp
  StringArray.hpp
  StringArrayBuilder.hpp
  SACs.hpp
  CodeArray.hpp
  Log.hpp
  MultiBitVector.hpp
  Vector.hpp
)

set(SOURCE_FILES
  ArrayFSABuilder.cpp
  ArrayFSATailBuilder.cpp
  PlainFSABuilder.cpp
  StringDictBuilder.cpp
  FSA.cpp
  StringTransFSA.cpp
)

add_library(ArrayFSA STATIC
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

add_executable(test_PlainFSA test_PlainFSA.cpp)
target_link_libraries(test_PlainFSA ArrayFSA)

add_executable(bench bench.cpp FsaTools.hpp)
target_link_libraries(bench ArrayFSA)

add_executable(build build.cpp FsaGenerator.hpp)
target_link_libraries(build ArrayFSA)

add_executable(makefsa makefsa.cpp FsaGenerator.hpp)
target_link_libraries(makefsa ArrayFSA)

add_executable(codes codes.cpp CodesMeasurer.hpp)
target_link_libraries(codes ArrayFSA)

add_executable(build_array build_array.cpp)
target_link_libraries(build_array ArrayFSA)

