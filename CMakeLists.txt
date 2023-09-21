cmake_minimum_required(VERSION 3.0.0)
project(css_parser_c VERSION 0.1.0 LANGUAGES C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CTest)
enable_testing()
add_subdirectory(tests)

add_subdirectory(css_parser)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
