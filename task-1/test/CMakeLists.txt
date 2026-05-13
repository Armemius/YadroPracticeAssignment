FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    URL_HASH SHA256=40d4ec942217dcc84a9ebe2a68584ada7d4a33a8ee958755763278ea1c5e18ff
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

file(GLOB CORE_TESTS_SOURCES_LIST CONFIGURE_DEPENDS "unit/bot/*.cpp" "unit/core/*.cpp" "unit/parser/*.cpp")
add_executable(core_tests ${CORE_TESTS_SOURCES_LIST})
target_link_libraries(core_tests PRIVATE bot core parser GTest::gtest_main)
target_compile_features(core_tests PRIVATE cxx_std_20)

gtest_discover_tests(core_tests)
