#include <check.h>

#include "../css_parser/elements.h"

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
            is_valid: true,
            selectors_number: 2,
            declarations_number: 3,
        }
    };

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
            size_t selectors_number = 0;
            size_t declarations_number = 0;
            for (struct token* token = element.start; token != )
            for
        } else {
            ck_assert_int_eq(result, element_error);
        }
    }
}
END_TEST

Suite* test_elements_suite() {
    Suite* suite = suite_create("Elements");

    TCase* identifiers = tcase_create("Identifiers");
    tcase_add_test(identifiers, identifier_test);

    TCase* parse = tcase_create("Parse");
    // tcase_add_test(parse, parse_tokens_case1);
    tcase_add_test(parse, parse_tokens_invalid_token);

    suite_add_tcase(suite, identifiers);
    suite_add_tcase(suite, parse);

    return suite;
}