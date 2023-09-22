find_package(Check REQUIRED)

add_executable(tests tests.c)
target_link_libraries(tests ${CHECK_LIBRARIES} regex pthread)
target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic)