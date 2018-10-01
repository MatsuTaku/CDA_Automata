cmake_minimum_required(VERSION 3.1)

project(csd_automata)

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

file(GLOB HEADER_FILES ${IncludeDir}/csd_automata.hpp ${IncludeDir}/csd_automata/*.hpp)

# Build Interface Library
add_library(CSDAutomata STATIC ${IncludeDir}/csd_automata.cpp ${HEADER_FILES})
#target_sources(CSDAutomata PRIVATE ${HEADER_FILES})
target_include_directories(CSDAutomata PUBLIC ${IncludeDir})

add_subdirectory(lib/SimDS)
target_link_libraries(CSDAutomata PUBLIC sim_ds)
#get_target_property(SimDS_INCLUDE_DIR sim_ds PROPERTY ${INTERFACE_INCLUDE_DIRECTORIES})
#target_include_directories(CSDAutomata PRIVATE ${SimDS_INCLUDE_DIR})

link_directories(/usr/local/lib)
include_directories(/usr/local/include)


add_executable(test_PlainFSA test_PlainFSA.cpp)
target_link_libraries(test_PlainFSA CSDAutomata)

file(GLOB wrappers XcdatWrapper.hpp MarisaWrapper.hpp DartsCloneWrapper.hpp)

add_executable(bench bench.cpp ${wrappers})
target_link_libraries(bench CSDAutomata xcdat marisa)

add_executable(build build.cpp DABuilder.hpp)
target_link_libraries(build CSDAutomata)

add_executable(convert convert.cpp)
target_link_libraries(convert CSDAutomata)

add_executable(makefsa makefsa.cpp DABuilder.hpp)
target_link_libraries(makefsa CSDAutomata)

add_executable(build_array build_array.cpp)
target_link_libraries(build_array CSDAutomata)


# Build Static Library for Development on Xcode
#add_library(CSDAutomata_dev STATIC ${IncludeDir}/array_fsa.cpp ${HEADER_FILES})
#include_directories(${IncludeDir})
#target_link_libraries(CSDAutomata_dev PRIVATE sim_ds)


if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	message(STATUS "Enable test CSDAutomata")
	enable_testing()
	add_subdirectory(test)
endif()

