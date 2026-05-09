# Game core library

file(GLOB GAME_CORE_SOURCES_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/core/*.cpp")
file(GLOB GAME_CORE_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/core/*.hpp")
add_library(core ${GAME_CORE_SOURCES_LIST} ${GAME_CORE_HEADER_LIST})
target_include_directories(core PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_features(core PUBLIC cxx_std_20)

source_group(
    TREE "${PROJECT_SOURCE_DIR}/include"
    PREFIX "Core Header Files"
    FILES ${GAME_CORE_HEADER_LIST}
)
source_group(
    TREE "${PROJECT_SOURCE_DIR}/src"
    PREFIX "Core Source Files"
    FILES ${GAME_CORE_SOURCES_LIST}
)