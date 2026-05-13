# Simulation core library

file(GLOB_RECURSE SIMULATION_SOURCES_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/simulation/**.cpp")
file(GLOB_RECURSE SIMULATION_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/simulation/**.hpp")
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