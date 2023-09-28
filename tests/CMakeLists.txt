find_package(Check REQUIRED)

add_executable(tests tests.c)
target_link_libraries(tests ${CHECK_LIBRARIES} regex pthread css_parser_lib)
target_include_directories(tests PUBLIC ../css_parser/ C:/msys64/mingw64/include)