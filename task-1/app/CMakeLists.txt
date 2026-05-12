add_executable(task task.cpp)
target_compile_features(task PRIVATE cxx_std_20)
target_link_libraries(task PRIVATE core parser bot)

add_executable(optimized_task optimized_task.cpp)
target_compile_features(optimized_task PRIVATE cxx_std_20)
target_link_libraries(optimized_task PRIVATE core parser bot)
