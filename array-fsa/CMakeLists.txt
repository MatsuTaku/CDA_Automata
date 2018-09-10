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

# Build Interface Library
add_library(ArrayFSA INTERFACE)
target_sources(ArrayFSA INTERFACE ${HEADER_FILES})
target_include_directories(ArrayFSA INTERFACE ${IncludeDir})

add_subdirectory(lib/SimDS)
target_link_libraries(ArrayFSA INTERFACE sim_ds)
#get_target_property(SimDS_INCLUDE_DIR sim_ds PROPERTY ${INTERFACE_INCLUDE_DIRECTORIES})

link_directories(/usr/local/lib)
include_directories(/usr/local/include)

add_executable(test_PlainFSA test_PlainFSA.cpp)
target_link_libraries(test_PlainFSA ArrayFSA)

file(GLOB wrappers XcdatWrapper.hpp)

add_executable(bench bench.cpp FsaTools.hpp ${wrappers})
target_link_libraries(bench ArrayFSA xcdat marisa)

add_executable(build build.cpp FsaGenerator.hpp)
target_link_libraries(build ArrayFSA)

add_executable(makefsa makefsa.cpp FsaGenerator.hpp)
target_link_libraries(makefsa ArrayFSA)

add_executable(build_array build_array.cpp)
target_link_libraries(build_array ArrayFSA)


# Build Static Library for Development on Xcode
add_library(ArrayFSA_dev STATIC ${IncludeDir}/array_fsa.cpp ${HEADER_FILES})
include_directories(${IncludeDir})
target_link_libraries(ArrayFSA_dev PRIVATE sim_ds)


if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	message(STATUS "Enable test ArrayFSA")
	enable_testing()
	add_subdirectory(test)
endif()

