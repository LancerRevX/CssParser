add_executable(css_parser main.c)
target_link_libraries(css_parser regex)
target_compile_options(css_parser PRIVATE -Wall -Wextra -Wpedantic)