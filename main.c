#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum token_type {
    space,
    comment
};

struct token {
    enum token_type type;
    const char* pointer;
    size_t length;
};

struct token* parse_tokens(char const* source)
{
    size_t source_len = strlen(source);
    struct token tokens[source_len];

    regex_t space_regex;
    int space_regex_comp_result = regcomp(&space_regex, "^[:space:]+", 0);
    if (space_regex_comp_result != 0) {
        fprintf(stderr, "Error compiling space regex. Error code = %d", space_regex_comp_result);
        exit(space_regex_comp_result);
    }

    size_t i = 0;
    while (i < source_len) {
        regmatch_t space_match;
            regexec(&space_regex, &source[i], 1, &space_match, 0);

    }
}

int get_single_char_token(char const* source, size_t pos) {
    
}

int main(int, char**)
{
    printf("Hello, from css_parser_c!\n");
}
