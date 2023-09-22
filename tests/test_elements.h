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

Suite* test_elements_suite() {
    Suite* suite = suite_create("Elements");

    TCase* rule_sets = tcase_create("Rule set");
    tcase_add_test(rule_sets, rule_set_valid);

    return suite;
}