add_subdirectory(css)

add_executable(css_parser main.c)
target_link_libraries(css_parser css_parser_lib regex)
target_compile_options(css_parser PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Werror -Wno-error=unused-but-set-variable -Wno-error=unused-parameter -O1)

target_include_directories(css_parser PUBLIC .)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")