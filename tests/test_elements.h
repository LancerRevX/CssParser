#include <check.h>

#include "css/elements.h"

enum element_status parse_elements_from_source(char const* source, struct element** first_element, struct syntax_error* error) {
}

START_TEST(rule_set_valid) {
    struct {
        char const* source;
        bool is_valid;
        size_t selectors_number;
        size_t declarations_number;
    } tests[10] = {
        {
            "#some-selector .chlid-selector,"
            "selector-2 > selector2-child {",
            "   property1: value1;"
            "   proerty2_: url(some url);",
            "   property3: 'a string with unmatched parenthesis (  123';"
            "}",
            is_valid : true,
            selectors_number : 2,
            declarations_number : 3,
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
    enum token_status token_status = parse_tokens(&first_token, &tokens_number, source, &lexical_error);
    ck_assert_int_eq(token_status, token_found);

    struct element at_rule;
    struct syntax_error syntax_error;
    enum element_status at_rule_result = parse_at_rule(&first_token, &at_rule, &syntax_error);

    ck_assert_int_eq(at_rule_result, element_found);
    ck_assert_int_eq(at_rule.type, element_regular_at_rule);
    ck_assert_int_eq(element_length(&at_rule), 16);
    ck_assert_int_eq(at_rule.start->type, token_at);
    ck_assert_int_eq(at_rule.end->type, token_string);
}
END_TEST

START_TEST(test_valid_declaration) {
    char const* source = "   \t   \n /*abc  */  some-property  :   url (\"some-url\")      ;";
    struct token* first_token = 0;
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status token_status = parse_tokens(&first_token, &tokens_number, source, &lexical_error);
    ck_assert_int_eq(token_status, token_found);

    struct element declaration;
    struct syntax_error syntax_error;
    enum element_status declaration_result = parse_declaration(&first_token, &declaration, &syntax_error);

    ck_assert_int_eq(declaration_result, element_found);
    ck_assert_int_eq(declaration.type, element_declaration);
    ck_assert_int_eq(element_length(&declaration), strlen("some-property  :   url (\"some-url\")"));
    ck_assert_int_eq(element_token_length(&declaration), 9);
    ck_assert_int_eq(declaration.start->type, token_identifier);
    ck_assert_int_eq(declaration.end->type, token_parentheses_end);

    struct element* property = declaration.first_child;
    ck_assert_int_eq(property->type, element_property);
    ck_assert_int_eq(element_length(property), strlen("some-property"));

    struct element* value = property->next;
    ck_assert_int_eq(value->type, element_value);
}
END_TEST

START_TEST(test_valid_property) {
    struct {
        char const* source;
        size_t tokens_number;
        size_t element_length;
    } sources[] = {
        {
            source: "            a-simple-property                ",
            tokens_number: 1,
            element_length: strlen("a-simple-property"),
        },
        {
            source: "   \t   \n /*abc  */  url (\"some-url\")      ;",
            tokens_number: 1,
            element_length: strlen("url"),
        },
        {
            source: "some-value,  \t   \n /*abc  */ 'and some more'      ;",
            tokens_number: 1,
            element_length: strlen("some-value"),
        },
        {0, 0, 0}};
    for (size_t i = 0; sources[i].source != 0; i++) {
        struct token* first_token = 0;
        size_t tokens_number = 0;
        struct lexical_error lexical_error;
        enum token_status result = parse_tokens(&first_token, &tokens_number, sources[i].source, &lexical_error);
        ck_assert_int_eq(result, token_found);

        struct element property;
        struct syntax_error syntax_error;
        enum element_status property_result = parse_property(first_token, &property, &syntax_error);

        ck_assert_int_eq(property_result, element_found);
        ck_assert_int_eq(property.type, element_property);
        ck_assert_int_eq(element_length(&property), sources[i].element_length);
        ck_assert_int_eq(element_token_length(&property), sources[i].tokens_number);
    }
}
END_TEST

START_TEST(test_valid_value) {
    struct {
        char const* source;
        size_t tokens_number;
        size_t element_length;
    } sources[] = {
        {
            source: "            a-simple-value                ",
            tokens_number: 1,
            element_length: strlen("a-simple-value"),
        },
        {
            source: "   \t   \n /*abc  */  url (\"some-url\")      ;",
            tokens_number: 5,
            element_length: strlen("url (\"some-url\")"),
        },
        {
            source: "some-value,  \t   \n /*abc  */ 'and some more'      ;",
            tokens_number: 5,
            element_length: strlen("url (\"some-url\")"),
        },
        {0, 0, 0}};
    for (size_t i = 0; sources[i].source != 0; i++) {
        struct token* first_token = 0;
        size_t tokens_number = 0;
        struct lexical_error lexical_error;
        enum token_status result = parse_tokens(&first_token, &tokens_number, sources[i].source, &lexical_error);
        ck_assert_int_eq(result, token_found);

        struct element value;
        struct syntax_error syntax_error;
        enum element_status value_result = parse_value(first_token, &value, &syntax_error);

        ck_assert_int_eq(value_result, element_found);
        ck_assert_int_eq(value.type, element_value);
        ck_assert_int_eq(element_length(&value), sources[i].element_length);
        ck_assert_int_eq(element_token_length(&value), sources[i].tokens_number);
    }
}
END_TEST

// START_TEST(test_valid_declaration) {
//     char const* source = "   \t   \n /*abc  */  some-property  :   url (\"some-url\")      ;";
//     struct token* first_token = 0;
//     size_t tokens_number = 0;
//     struct lexical_error lexical_error;
//     enum token_status token_status = parse_tokens(&first_token, &tokens_number, source, &lexical_error);
//     ck_assert_int_eq(token_status, token_found);

//     struct element declaration;
//     struct syntax_error syntax_error;
//     enum element_status declaration_result = parse_declaration(&first_token, &declaration, &syntax_error);

//     ck_assert_int_eq(declaration_result, element_found);
//     ck_assert_int_eq(declaration.type, element_declaration);
//     ck_assert_int_eq(element_length(&declaration), strlen("some-property  :   url (\"some-url\")"));
//     ck_assert_int_eq(element_token_length(&declaration), 9);
//     ck_assert_int_eq(declaration.start->type, token_identifier);
//     ck_assert_int_eq(declaration.end->type, token_parentheses_end);

//     struct element* property = declaration.first_child;
//     ck_assert_int_eq(property->type, element_property);
//     ck_assert_int_eq(element_length(property), strlen("some-property"));

//     struct element* value = property->next;
//     ck_assert_int_eq(value->type, element_value);
// }
// END_TEST

Suite* test_elements_suite() {
    Suite* suite = suite_create("Elements");

    // TCase* rule_sets = tcase_create("Rule set");
    // tcase_add_test(rule_sets, rule_set_valid);

    TCase* common = tcase_create("Common element");
    tcase_add_test(common, test_valid_property);
    tcase_add_test(common, test_valid_value);

    TCase* at_rules = tcase_create("At rules");
    tcase_add_test(at_rules, test_at_rule_rule);
    tcase_add_test(at_rules, test_valid_declaration);

    // suite_add_tcase(suite, rule_sets);
    suite_add_tcase(suite, common);
    suite_add_tcase(suite, at_rules);

    return suite;
}