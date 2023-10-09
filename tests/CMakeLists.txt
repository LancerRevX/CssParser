add_executable(test_tokens test_tokens.c)
target_link_libraries(test_tokens css_parser_lib cmocka)
target_include_directories(test_tokens PUBLIC ../css_parser/)

add_executable(test_elementss test_elements.c)
target_link_libraries(test_elementss css_parser_lib cmocka)
target_include_directories(test_elementss PUBLIC ../css_parser/)