find_package(Check REQUIRED)

add_executable(test_tokens test_tokens.c)
target_link_libraries(test_tokens ${CHECK_LIBRARIES} regex pthread)
target_compile_options(test_tokens PRIVATE -Wall -Wextra -Wpedantic -Werror)