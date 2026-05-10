add_executable(task task.cpp)
target_compile_features(task PRIVATE cxx_std_20)
target_link_libraries(task PRIVATE core parser bot)
