add_subdirectory(css)

add_executable(css_parser main.c)
target_link_libraries(css_parser css_parser_lib regex)

target_include_directories(css_parser PUBLIC .)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")