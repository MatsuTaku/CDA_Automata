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
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
    endif()
endif()

set(Daram_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/../daram/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../daram/lib/SimpleDataStructure/include"
)

set(SUCCINCT_USE_INTRINSICS ON)
set(SUCCINCT_USE_POPCNT ON)
add_subdirectory(lib/path_decomposed_tries/succinct EXCLUDE_FROM_ALL)

find_package(Boost 1.53.0 REQUIRED
    COMPONENTS unit_test_framework iostreams system thread filesystem)

include_directories(include ${Daram_INCLUDE_DIRS} /usr/local/include lib/path_decomposed_tries ${Boost_INCLUDE_DIRS})
link_directories(/usr/local/lib ${Boost_LIBRARY_DIRS})

set(CLANG_WARNING_OPTIONS -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast)
set(GCC_WARNING_OPTIONS -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused)
set(MSVC_WARNING_OPTIONS /W4)
set(WARNING_OPTIONS
    $<$<CXX_COMPILER_ID:AppleClang>:${CLANG_WARNING_OPTIONS}>
    $<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNING_OPTIONS}>
    $<$<CXX_COMPILER_ID:GCC>:${GCC_WARNING_OPTIONS}>
    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNING_OPTIONS}>
    )

message(STATUS "Make SDExperiments")
message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE are ${CMAKE_CXX_FLAGS_RELEASE}")

file(GLOB WRAPPERS include/*Wrapper.hpp)
add_executable(sd_experiments src/experiment.cpp ${WRAPPERS})
target_link_libraries(sd_experiments xcdat marisa succinct ${Boost_LIBRARIES})
target_compile_options(sd_experiments PRIVATE ${WARNING_OPTIONS})

add_executable(darts_build src/darts_build.cpp include/DartsCloneWrapper.hpp)
target_compile_options(darts_build PRIVATE ${WARNING_OPTIONS})

add_executable(samc_build src/samc_build.cpp include/SamcWrapper.hpp)
target_compile_options(samc_build PRIVATE ${WARNING_OPTIONS})

