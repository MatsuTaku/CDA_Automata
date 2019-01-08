add_executable(daram_build daram_build.cpp)
target_link_libraries(daram_build CSDAutomata)

add_executable(daram_benchmark daram_benchmark.cpp)
target_link_libraries(daram_benchmark CSDAutomata)

add_executable(convert convert.cpp)
target_link_libraries(convert CSDAutomata)

