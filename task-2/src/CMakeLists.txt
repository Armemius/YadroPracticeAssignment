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

# Parser library

file(GLOB PARSER_SOURCES_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/parser/*.cpp")
file(GLOB PARSER_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/parser/*.hpp")
add_library(parser ${PARSER_SOURCES_LIST} ${PARSER_HEADER_LIST})
target_link_libraries(parser PUBLIC simulation)
target_include_directories(parser PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_features(parser PUBLIC cxx_std_20)

source_group(
    TREE "${PROJECT_SOURCE_DIR}/include"
    PREFIX "Parser Header Files"
    FILES ${PARSER_HEADER_LIST}
)
source_group(
    TREE "${PROJECT_SOURCE_DIR}/src"
    PREFIX "Parser Source Files"
    FILES ${PARSER_SOURCES_LIST}
)
