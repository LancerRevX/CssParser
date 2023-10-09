#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "css/tokens.h"

void test_space(void** state) {
    (void)state;

    char const* source = "   \n\t/*abc*/  defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_space_token(&token, source, 0, &error);

    assert_int_equal(result, token_found);
    assert_int_equal(token.type, token_space);
    assert_int_equal(token.length, 5);
    assert_ptr_equal(token.pointer, &source[0]);
}

void test_valid_comment(void** state) {
    (void)state;

    char const* source = "/*abc*/  defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_comment_token(&token, source, 0, &error);

    assert_int_equal(result, token_found);
    assert_int_equal(token.type, token_comment);
    assert_int_equal(token.length, 7);
    assert_ptr_equal(token.pointer, &source[0]);
}

void test_absent_comment(void** state) {
    (void)state;

    char const* source = "hijklm /*abc*/  defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_comment_token(&token, source, 0, &error);

    assert_int_equal(result, token_not_found);
}

void test_unclosed_comment(void** state) {
    (void)state;

    char const* source = "/*abc defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_comment_token(&token, source, 0, &error);

    assert_int_equal(result, token_error);
    assert_ptr_equal(error.source, source);
    assert_int_equal(error.pos, 0);
    assert_int_equal(error.length, 2);
}

void test_identifier(void** state) {
    (void)state;

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

        assert_true(identifier_sources[i].found ^ (result != token_found));
        if (identifier_sources[i].found) {
            assert_int_equal(token.type, token_identifier);
            assert_int_equal(token.length, strlen(identifier_sources[i].identifier));
            assert_int_equal(strncmp(token.pointer, identifier_sources[i].identifier, token.length), 0);
        }
    }
}

void test_string(void** state) {
    (void)state;

    static const struct {
        char const* source;
        bool valid;
        size_t index;
        char const* string;
    } string_tests[] = {
        {
            .source = "   \n \t \"quoted-string   ;\"  ;",
            .valid = true,
            .index = 1,
            .string = "\"quoted-string   ;\"",
        },
        {
            .source = "'some string \" with double quote inside ' ",
            .valid = true,
            .index = 0,
            .string = "'some string \" with double quote inside '",
        },
        {.source = " 'invalid string (unclosed) ",
         .valid = false,
         .index = 0,
         .string = 0}};

    for (size_t _i = 0; _i < sizeof(string_tests) / sizeof(*string_tests); _i++) {
        struct token* tokens = calloc(strlen(string_tests[_i].source), sizeof(struct token));
        size_t tokens_number;
        struct lexical_error error;
        enum token_status result = parse_tokens(tokens, &tokens_number, string_tests[_i].source, &error);

        if (string_tests[_i].valid) {
            assert_int_equal(result, token_found);
            struct token* token = &tokens[string_tests[_i].index];
            assert_int_equal(token->type, token_string);
            assert_string_equal(token->string, string_tests[_i].string);
        } else {
            assert_int_equal(result, token_error);
        }
    }
}

void test_tokens(void** state) {
    (void)state;

    char const* source = "/* abc comment */"                  // 1
                         "selector,another_selector{"         // 4
                         "property:value;"                    // 4
                         "another_property:\"quoted-value\";" // 4
                         "}";                                 // 1
    struct token* tokens = calloc(strlen(source), sizeof(struct token));
    size_t tokens_number;
    struct lexical_error error;
    enum token_status result = parse_tokens(tokens, &tokens_number, source, &error);

    assert_int_equal(result, token_found);
    assert_int_equal(tokens_number, 14);
}

void test_invalid_token(void** state) {
    (void)state;

    char const* source = "/* abc comment */"
                         "selector,another_selector?{" // $ <--
                         "property:value;"
                         "}";
    struct token* tokens = calloc(strlen(source), sizeof(struct token));
    size_t tokens_number;
    struct lexical_error error;
    enum token_status result = parse_tokens(tokens, &tokens_number, source, &error);

    assert_int_equal(result, token_error);
    assert_int_equal(error.pos, strstr(source, "?") - source);
}

void test_number(void** state) {
    (void)state;

    static const struct {
        char const* source;
        bool valid;
        char const* number;
    } tests[] = {
        {
            .source = "abc",
            .valid = false,
        },
        {
            .source = "123px",
            .valid = true,
            .number = "123",
        },
        {
            .source = "123.689px",
            .valid = true,
            .number = "123.689",
        },
        {
            .source = "123.689%",
            .valid = true,
            .number = "123.689",
        },
        {
            .source = "123..689%",
            .valid = false,
        },
        {
            .source = ".689%",
            .valid = true,
            .number = ".689"
        },
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(*tests); i++) {
        struct token token;
        struct lexical_error error;
        enum token_status result = get_number_token(&token, tests[i].source, 0, &error);
        if (tests[i].valid) {
            assert_int_equal(result, token_found);
            assert_int_equal(token.type, token_number);
            assert_string_equal(token.string, tests[i].number);
        } else {
            assert_int_not_equal(result, token_found);
        }
    }
}

static const struct CMUnitTest token_tests[] = {
    cmocka_unit_test(test_space),
    cmocka_unit_test(test_valid_comment),
    cmocka_unit_test(test_absent_comment),
    cmocka_unit_test(test_unclosed_comment),
    cmocka_unit_test(test_identifier),
    cmocka_unit_test(test_string),
    cmocka_unit_test(test_tokens),
    cmocka_unit_test(test_invalid_token),
    cmocka_unit_test(test_number),
};

int main(void) {
    cmocka_run_group_tests(token_tests, NULL, NULL);
    // return 0;
}
