cmake_minimum_required(VERSION 3.14)
project(ProductionFacilitySimulation VERSION 0.0.1 LANGUAGES C CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    include(CTest)
endif()

include(FetchContent)

# Simulation core library

file(GLOB_RECURSE SIMULATION_SOURCES_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/**.cpp")
file(GLOB_RECURSE SIMULATION_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/**.hpp")
add_library(simulation ${SIMULATION_SOURCES_LIST} ${SIMULATION_HEADER_LIST})
target_include_directories(simulation PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_features(simulation PUBLIC cxx_std_20)

source_group(
    TREE "${PROJECT_SOURCE_DIR}/include"
    PREFIX "Simulation Header Files"
    FILES ${SIMULATION_HEADER_LIST}
)
source_group(
    TREE "${PROJECT_SOURCE_DIR}/src"
    PREFIX "Simulation Source Files"
    FILES ${SIMULATION_SOURCES_LIST}
)

add_subdirectory(app)
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND BUILD_TESTING)
    add_subdirectory(test)
endif()
