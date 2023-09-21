#include <stdlib.h>

#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include "../css_parser/tokens.h"

Test(space, valid) {
    char const* source = "   \n\t/*abc*/  defg";
    struct token token;
    enum token_status result = get_space_token(&token, source, 0);

    cr_assert(eq(int, result, token_found));
    cr_assert(eq(int, token.type, token_space));
    cr_assert(eq(int, token.length, 5));
    cr_assert(token.pointer == &source[0]);
}

Test(comment, valid) {
    char const* source = "/*abc*/  defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_comment_token(&token, source, 0, &error);

    cr_assert(eq(int, result, token_found));
    cr_assert(eq(int, token.type, token_comment));
    cr_assert(eq(int, token.length, 7));
    cr_assert(token.pointer == &source[0]);
}

Test(comment, absent) {
    char const* source = "hijklm /*abc*/  defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_comment_token(&token, source, 0, &error);

    cr_assert(eq(int, result, token_not_found));
}

Test(comment, unclosed) {
    char const* source = "/*abc defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_comment_token(&token, source, 0, &error);

    cr_assert(eq(int, result, token_error));
    cr_assert(error.source == source);
    cr_assert(eq(int, error.pos, 0));
    cr_assert(eq(int, error.length, 2));
}

Test(parse_tokens, case1) {
    char const* source = "\
        /* abc comment */\
        selector, another_selector {\
            property: value;\
        }\
    ";
    struct token* first_token;
    size_t tokens_number;
    struct lexical_error error;
    enum token_status result = parse_tokens(&first_token, &tokens_number, source, &error);

    cr_assert(eq(int, result, token_found));
    cr_assert(eq(int, tokens_number, 18));
}
