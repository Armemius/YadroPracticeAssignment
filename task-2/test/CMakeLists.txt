FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    URL_HASH SHA256=40d4ec942217dcc84a9ebe2a68584ada7d4a33a8ee958755763278ea1c5e18ff
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

file(GLOB SIMULATION_TESTS_SOURCES_LIST CONFIGURE_DEPENDS "machine_test.cpp")
add_executable(simulation_test ${SIMULATION_TESTS_SOURCES_LIST})
target_link_libraries(simulation_test PRIVATE simulation GTest::gtest_main)
target_compile_features(simulation_test PRIVATE cxx_std_20)

gtest_discover_tests(simulation_test)
