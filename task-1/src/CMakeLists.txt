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

# Parser library

file(GLOB PARSER_SOURCES_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/parser/*.cpp")
file(GLOB PARSER_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/parser/*.hpp")
add_library(parser ${PARSER_SOURCES_LIST} ${PARSER_HEADER_LIST})
target_link_libraries(parser PRIVATE core)
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

# Bot library

file(GLOB BOT_SOURCES_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/bot/*.cpp")
file(GLOB BOT_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/bot/*.hpp")
add_library(bot ${BOT_SOURCES_LIST} ${BOT_HEADER_LIST})
target_link_libraries(bot PRIVATE core)
target_include_directories(bot PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_features(bot PUBLIC cxx_std_20)

source_group(
    TREE "${PROJECT_SOURCE_DIR}/include"
    PREFIX "Bot Header Files"
    FILES ${BOT_HEADER_LIST}
)
source_group(
    TREE "${PROJECT_SOURCE_DIR}/src"
    PREFIX "Bot Source Files"
    FILES ${BOT_SOURCES_LIST}
)
