find_package(Criterion REQUIRED)

add_executable(test_tokens test_tokens.c)
target_link_libraries(test_tokens ${CRITERION_LIBRARIES} regex pthread)
target_compile_options(test_tokens PRIVATE -Wall -Wextra -Wpedantic -Werror)