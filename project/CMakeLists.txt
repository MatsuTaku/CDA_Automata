cmake_minimum_required(VERSION 3.12)

project(csd_automata)

set(CMAKE_CXX_STANDARD 17)

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
target_include_directories(CSDAutomata PUBLIC ${IncludeDir})

add_subdirectory(lib/SimpleDataStructure)
target_link_libraries(CSDAutomata PUBLIC sim_ds)


link_directories(/usr/local/lib)
include_directories(/usr/local/include)

add_subdirectory(tools)

enable_testing()
add_subdirectory(test)

