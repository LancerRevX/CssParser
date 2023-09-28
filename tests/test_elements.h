#include <check.h>

#include "css/elements.h"

START_TEST(rule_set_valid) {
    struct {
        char const* source;
        bool is_valid;
        size_t selectors_number;
        size_t declarations_number;
    } tests[10] = {
        {
            "#some-selector .chlid-selector,"
            "selector-2 > selector2-child {"
            "   property1: value1;"
            "   proerty2_: url(some url);"
            "   property3: 'a string with unmatched parenthesis (  123';"
            "}",
            .is_valid = true,
            .selectors_number = 2,
            .declarations_number = 3,
        }};

    for (size_t i = 0; i < 1; i++) {
        struct token* first_token;
        size_t tokens_number;
        struct lexical_error error;
        parse_tokens(&first_token, &tokens_number, tests[i].source, &error);

        struct element element;
        struct syntax_error syntax_error;
        int result = get_rule_set(&first_token, &element, &syntax_error);
        if (tests[i].is_valid) {
            ck_assert_int_eq(result, element_found);
            ck_assert_int_eq(element.type, element_rule_set);
            ck_assert_ptr_nonnull(element.first_child);
            ck_assert_int_eq(
                count_elements_of_type(&element, element_selector),
                tests[i].selectors_number
            );
            ck_assert_int_eq(
                count_elements_of_type(&element, element_declaration),
                tests[i].declarations_number
            );
        } else {
            ck_assert_int_eq(result, element_error);
        }
    }
}
END_TEST

START_TEST(test_at_rule_rule) {
    char const* source = "(max-width: 100px) { /* some rule sets */ }";
    struct token* first_token = 0;
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status token_result = parse_tokens(&first_token, &tokens_number, source, &lexical_error);
    ck_assert_int_eq(token_result, token_found);

    struct element rule;
    struct syntax_error syntax_error;
    enum element_status at_rule_result = parse_at_rule_rule(first_token, &rule, &syntax_error);

    ck_assert_int_eq(at_rule_result, element_found);
    ck_assert_int_eq(element_length(&rule), 18);
    ck_assert_int_eq(rule.start->type, token_parentheses_start);
    ck_assert_int_eq(rule.end->type, token_parentheses_end);
}
END_TEST

START_TEST(test_regular_at_rule) {
    char const* source = "@charset \"utf-8\"; something else ;";
    struct token* first_token = 0;
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status token_result = parse_tokens(&first_token, &tokens_number, source, &lexical_error);
    ck_assert_int_eq(token_result, token_found);

    struct element at_rule;
    struct syntax_error syntax_error;
    enum element_status at_rule_result = parse_at_rule(first_token, &at_rule, &syntax_error);

    ck_assert_int_eq(at_rule_result, element_found);
    ck_assert_int_eq(at_rule.type, element_regular_at_rule);
    ck_assert_int_eq(element_length(&at_rule), 16);
    ck_assert_int_eq(at_rule.start->type, token_at);
    ck_assert_int_eq(at_rule.end->type, token_string);
}
END_TEST

struct property_test {
    char const* source;
    char const* token_string;
};

static struct property_test property_tests[] = {
    {
        .source = "            a-simple-property                ",
        .token_string = "a-simple-property",
    },
    {
        .source = "   \t   \n /*abc  */  url (\"some-url\")      ;",
        .token_string = "url",
    },
    {
        .source = "some-value,  \t   \n /*abc  */ 'and some more'      ;",
        .token_string = "some-value",
    }};

START_TEST(test_valid_property) {
    struct token* first_token = 0;
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status result = parse_tokens(&first_token, &tokens_number, property_tests[_i].source, &lexical_error);
    ck_assert_int_eq(result, token_found);

    struct element property;
    struct syntax_error syntax_error;
    enum element_status property_result = parse_property(first_token, &property, &syntax_error);

    ck_assert_int_eq(property_result, element_found);
    ck_assert_int_eq(property.type, element_property);
    ck_assert_int_eq(element_token_length(&property), 1);
    ck_assert_int_eq(element_length(&property), strlen(property_tests[_i].token_string));
    ck_assert_str_eq(property.start->string, property_tests[_i].token_string);
}
END_TEST

struct value_test {
    char const* source;
    size_t tokens_number;
    size_t element_length;
    char const* start_token_string;
    char const* end_token_string;
};

struct value_test value_tests[] = {
    {
        .source = "            a-simple-value                ",
        .tokens_number = 1,
        .element_length = strlen("a-simple-value"),
        .start_token_string = "a-simple-value",
        .end_token_string = "a-simple-value",
    },
    {
        .source = "   \t   \n /*abc  */  url (\"some-url\")      ;",
        .tokens_number = 5,
        .element_length = strlen("url (\"some-url\")"),
        .start_token_string = "url",
        .end_token_string = ")",
    },
    {
        .source = "some-value,  \t   \n /*abc  */ 'and some more'      ;",
        .tokens_number = 6,
        .element_length = strlen("some-value,  \t   \n /*abc  */ 'and some more'"),
        .start_token_string = "some-value",
        .end_token_string = "'and some more'",
    }};

START_TEST(test_valid_value) {

    struct token* first_token = 0;
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status result = parse_tokens(&first_token, &tokens_number, value_tests[_i].source, &lexical_error);
    ck_assert_int_eq(result, token_found);

    struct element value;
    struct syntax_error syntax_error;
    enum element_status value_result = parse_value(first_token, &value, &syntax_error);

    ck_assert_int_eq(value_result, element_found);
    ck_assert_int_eq(value.type, element_value);
    ck_assert_int_eq(element_token_length(&value), value_tests[_i].tokens_number);
    ck_assert_int_eq(element_length(&value), value_tests[_i].element_length);
    ck_assert_str_eq(value.start->string, value_tests[_i].start_token_string);
    ck_assert_str_eq(value.end->string, value_tests[_i].end_token_string);
}
END_TEST

struct declaration_test {
    char const* source;
    size_t declaration_length;
    char const* property_string;
    size_t value_length;
};

static struct declaration_test declaration_tests[] = {
    {
        .source = "   \n \t   some-property    :    some-value    ( nested \n value   \t );   abc",
        .declaration_length = strlen("some-property    :    some-value    ( nested \n value   \t )"),
        .property_string = "some-property",
        .value_length = strlen("some-value    ( nested \n value   \t )"),
    },
    {
        .source = "background-color: red",
        .declaration_length = strlen("background-color: red"),
        .property_string = "background-color",
        .value_length = strlen("red"),
    },
};

START_TEST(test_valid_declaration) {
    struct token* first_token = 0;
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status token_result = parse_tokens(&first_token, &tokens_number, declaration_tests[_i].source, &lexical_error);
    ck_assert_int_eq(token_result, token_found);

    struct element declaration;
    struct syntax_error syntax_error;
    enum element_status declaration_result = parse_declaration(first_token, &declaration, &syntax_error);

    ck_assert_msg(declaration_result == element_found, "parse declaration error: %s; token: %s", syntax_error.message, syntax_error.token->string);

    ck_assert_int_eq(declaration.type, element_declaration);
    ck_assert_int_eq(element_length(&declaration), declaration_tests[_i].declaration_length);

    struct element* property = declaration.first_child;
    ck_assert_int_eq(property->type, element_property);
    ck_assert_int_eq(element_length(property), strlen(declaration_tests[_i].property_string));
    ck_assert_str_eq(property->start->string, declaration_tests[_i].property_string);

    struct element* value = property->next;
    ck_assert_int_eq(value->type, element_value);
    ck_assert_int_eq(element_length(value), declaration_tests[_i].value_length);
}
END_TEST

Suite* get_elements_test_suite(void) {
    Suite* suite = suite_create("Elements");

    // TCase* rule_sets = tcase_create("Rule set");
    // tcase_add_test(rule_sets, rule_set_valid);

    TCase* common = tcase_create("Common");
    tcase_add_loop_test(common, test_valid_property, 0, sizeof(property_tests) / sizeof(struct property_test));
    tcase_add_loop_test(common, test_valid_value, 0, sizeof(value_tests) / sizeof(struct value_test));
    tcase_add_loop_test(common, test_valid_declaration, 0, sizeof(declaration_tests) / sizeof(struct declaration_test));

    TCase* at_rules = tcase_create("At rules");
    tcase_add_test(at_rules, test_at_rule_rule);
    // tcase_add_loop_test(at_rules, test_declarations, 0, DECLARATION_TESTS_NUMBER);

    // suite_add_tcase(suite, rule_sets);
    suite_add_tcase(suite, common);
    suite_add_tcase(suite, at_rules);

    return suite;
}
