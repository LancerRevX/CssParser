#include <stdlib.h>

#include <check.h>

#include "css/tokens.h"

START_TEST(space_valid) {
    char const* source = "   \n\t/*abc*/  defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_space_token(&token, source, 0, &error);

    ck_assert_int_eq(result, token_found);
    ck_assert_int_eq(token.type, token_space);
    ck_assert_int_eq(token.length, 5);
    ck_assert(token.pointer == &source[0]);
}
END_TEST

START_TEST(comment_valid) {
    char const* source = "/*abc*/  defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_comment_token(&token, source, 0, &error);

    ck_assert_int_eq(result, token_found);
    ck_assert_int_eq(token.type, token_comment);
    ck_assert_int_eq(token.length, 7);
    ck_assert(token.pointer == &source[0]);
}
END_TEST

START_TEST(comment_absent) {
    char const* source = "hijklm /*abc*/  defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_comment_token(&token, source, 0, &error);

    ck_assert_int_eq(result, token_not_found);
}
END_TEST

START_TEST(comment_unclosed) {
    char const* source = "/*abc defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_comment_token(&token, source, 0, &error);

    ck_assert_int_eq(result, token_error);
    ck_assert(error.source == source);
    ck_assert_int_eq(error.pos, 0);
    ck_assert_int_eq(error.length, 2);
}
END_TEST

START_TEST(identifier_test) {
    struct {
        char const* source;
        bool found;
        char const* identifier;
    } identifier_sources[5] = {
        {"/*abc defg", false, 0},
        {"--var-name1234 5asadf", true, "--var-name1234"},
        {"-1var-name1234 5asadf", false, 0},
        {"7var-name1234 5asadf", false, 0},
    };

    for (size_t i = 0; i < 2; i++) {
        struct token token;
        struct lexical_error error;
        token_status result = get_token(&token, identifier_sources[i].source, 0, &error);

        ck_assert(identifier_sources[i].found ^ (result != token_found));
        if (identifier_sources[i].found) {
            ck_assert_int_eq(token.type, token_identifier);
            ck_assert_int_eq(token.length, strlen(identifier_sources[i].identifier));
            ck_assert_int_eq(strncmp(token.pointer, identifier_sources[i].identifier, token.length), 0);
        }
    }
}
END_TEST

START_TEST(parse_tokens_case1) {
    char const* source = "/* abc comment */" // 1
        "selector,another_selector{" // 4
            "property:value;" // 4
            "another_property:\"quoted-value\";" // 6 (2 quotes)
    "}"; // 1
    struct token* first_token;
    size_t tokens_number;
    struct lexical_error error;
    enum token_status result = parse_tokens(&first_token, &tokens_number, source, &error);

    ck_assert_int_eq(result, token_found);
    ck_assert_int_eq(tokens_number, 16);
}
END_TEST


START_TEST(parse_tokens_invalid_token) {
    char const* source = "/* abc comment */"
        "selector,another_selector${" // $ <--
            "property:value;"
    "}";
    struct token* first_token;
    size_t tokens_number;
    struct lexical_error error;
    enum token_status result = parse_tokens(&first_token, &tokens_number, source, &error);

    ck_assert_int_eq(result, token_not_found);
    ck_assert_int_eq(error.pos, strstr(source, "$") - source);
}
END_TEST

Suite* test_tokens_suite() {
    Suite* suite = suite_create("Tokens");

    TCase* identifiers = tcase_create("Identifiers");
    tcase_add_test(identifiers, identifier_test);

    TCase* parse = tcase_create("Parse");
    // tcase_add_test(parse, parse_tokens_case1);
    tcase_add_test(parse, parse_tokens_invalid_token);

    suite_add_tcase(suite, identifiers);
    suite_add_tcase(suite, parse);

    return suite;
}
