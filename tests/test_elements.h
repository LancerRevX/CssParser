#include <check.h>

#include "css/elements.h"

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
    },
};

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
    },
};

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

struct declaration_block_test {
    char const* source;
    size_t declarations_number;
};

static struct declaration_block_test declaration_block_tests[] = {
    {
        .source = "    {  "
                  " \t some-property: some-value ( 'quoted string' something) \n"
                  "  }   ",
        .declarations_number = 1,
    },
    {
        .source = "  {a-property: a-value;"
                  "property-2: url('somewhere.com')"
                  "}",
        .declarations_number = 2,
    },
};

START_TEST(test_declaration_block) {
    struct token* first_token = 0;
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status token_result = parse_tokens(&first_token, &tokens_number, declaration_block_tests[_i].source, &lexical_error);
    ck_assert_int_eq(token_result, token_found);

    struct element declaration_block;
    struct syntax_error syntax_error;
    enum element_status declaration_block_result = parse_declaration_block(first_token, &declaration_block, &syntax_error);

    ck_assert_msg(declaration_block_result == element_found, "parse error: %s; token: %s", syntax_error.message, syntax_error.token->string);
    ck_assert_int_eq(declaration_block.type, element_declaration_block);

    ck_assert_int_eq(declaration_block.start->type, token_block_start);
    ck_assert_int_eq(declaration_block.end->type, token_block_end);
    ck_assert_int_eq(element_count_children(&declaration_block), declaration_block_tests[_i].declarations_number);
}
END_TEST

struct selector_list_test {
    char const* source;
    bool valid;
    size_t selectors_number;
};

static struct selector_list_test selector_list_tests[] = {
    {
        .source = "abc, selector-2, #some-id > .some-class",
        .valid = true,
        .selectors_number = 3,
    },
};

START_TEST(test_selector_list) {
    struct token* first_token = 0;
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status token_result = parse_tokens(&first_token, &tokens_number, selector_list_tests[_i].source, &lexical_error);
    ck_assert_int_eq(token_result, token_found);

    struct element selector_list;
    struct syntax_error syntax_error;
    enum element_status selector_list_result = parse_selector_list(first_token, &selector_list, &syntax_error);

    if (selector_list_tests[_i].valid) {
        ck_assert_msg(selector_list_result == element_found, "parse error: %s; token: %s", syntax_error.message, syntax_error.token->string);

        ck_assert_int_eq(selector_list.type, element_selector_list);
        ck_assert_int_eq(element_count_children(&selector_list), selector_list_tests[_i].selectors_number);
    } else {
        ck_assert_int_eq(selector_list_result, element_error);
    }
}
END_TEST

struct rule_set_test {
    char const* source;
    bool valid;
    size_t selectors_number;
    size_t declarations_number;
};

struct rule_set_test rule_set_tests[] = {
    {
        .source = "#some-selector-1,selector-2>selector-2-child\tanother-child \n"
                  "  {"
                  "some-property-1\n"
                  ":some-value"
                  ";"
                  "background: red;"
                  "background-image: url('somewhere.com');"
                  "}",
        .valid = true,
        .selectors_number = 2,
        .declarations_number = 3,
    },
    {
        .source = "#my-block {"
                  " my-property: my-value;"
                  " calc-prop: calc(var(--some-var) + var(--some-value) / 14.0)"
                  "}",
        .valid = true,
        .selectors_number = 1,
        .declarations_number = 2,
    },
    {
        .source = ".my-button {"
                  "display: flex;"
                  "flex-direction: row;"
                  "justify-content: center;"
                  "align-items: center;"
                  "gap: 0.4em;"
                  "max-width: 100%;"
                  "padding: 14px 20px;"
                  "box-sizing: border-box;"
                  "text-transform: uppercase;"
                  "background-color: #f3f3f3;"
                  "color: #3e3e3e; "
                  "font-size: 13px;"
                  "font-weight: 600;"
                  "line-height: 18px;"
                  "text-align: center;"
                  "}",
        .valid = true,
        .selectors_number = 1,
        .declarations_number = 15,
    },
    {
        .source = ".my-button {"
                  "display: flex;"
                  "flex-direction: row;"
                  "justify-content: center;"
                  "align-items: center;"
                  "gap: 0.4em;"
                  "max-width: 100%;"
                  "padding: 14px 20px;"
                  "box-sizing: border-box;"
                  "text-transform: uppercase;"
                  "background-color: #f3f3f3;"
                  "color: #3e3e3e; "
                  "font-size: 13px;"
                  "font-weight: 600;"
                  "line-height: 18px;"
                  "text-align: center;"
                  "", // <--
        .valid = false,
    },
    {
        .source = ".my-button " // <--
                  "display: flex;"
                  "flex-direction: row;"
                  "justify-content: center;"
                  "align-items: center;"
                  "gap: 0.4em;"
                  "max-width: 100%;"
                  "padding: 14px 20px;"
                  "box-sizing: border-box;"
                  "text-transform: uppercase;"
                  "background-color: #f3f3f3;"
                  "color: #3e3e3e; "
                  "font-size: 13px;"
                  "font-weight: 600;"
                  "line-height: 18px;"
                  "text-align: center;"
                  "}",
        .valid = false,
    },
    {
        .source = "\n \t   {" // <--
                  "display: flex;"
                  "flex-direction: row;"
                  "justify-content: center;"
                  "align-items: center;"
                  "gap: 0.4em;"
                  "max-width: 100%;"
                  "padding: 14px 20px;"
                  "box-sizing: border-box;"
                  "text-transform: uppercase;"
                  "background-color: #f3f3f3;"
                  "color: #3e3e3e; "
                  "font-size: 13px;"
                  "font-weight: 600;"
                  "line-height: 18px;"
                  "text-align: center;"
                  "}",
        .valid = false,
    },
    {
        .source = ".selector {"
                  "display: flex;"
                  "flex-direction: row;"
                  "justify-content: center;"
                  "align-items: center;"
                  "gap: 0.4em;"
                  "max-width: 100%;"
                  "padding: 14px 20px;"
                  "box-sizing: border-box;"
                  "text-transform: uppercase;"
                  "background-color: #f3f3f3;"
                  "color: #3e3e3e; "
                  "font-size: 13px;"
                  "font-weight: 600;"
                  "line-height: 18px" // <--
                  "text-align: center;"
                  "}",
        .valid = false,
    },
};

START_TEST(test_rule_set) {
    struct token* first_token = 0;
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status token_result = parse_tokens(&first_token, &tokens_number, rule_set_tests[_i].source, &lexical_error);
    ck_assert_int_eq(token_result, token_found);

    struct element rule_set;
    struct syntax_error syntax_error;
    enum element_status rule_set_result = parse_rule_set(first_token, &rule_set, &syntax_error);

    if (rule_set_tests[_i].valid) {
        ck_assert_msg(rule_set_result == element_found, "parse error: %s; token: %s", syntax_error.message, syntax_error.token->string);
        ck_assert_int_eq(rule_set.type, element_rule_set);
        ck_assert_int_eq(element_count_children(&rule_set), 2);

        struct element* selector_list = rule_set.first_child;
        ck_assert_int_eq(selector_list->type, element_selector_list);
        ck_assert_int_eq(element_count_children(selector_list), rule_set_tests[_i].selectors_number);

        struct element* declaration_block = selector_list->next;
        ck_assert_int_eq(declaration_block->type, element_declaration_block);
        ck_assert_int_eq(element_count_children(declaration_block), rule_set_tests[_i].declarations_number);

        ck_assert_ptr_eq(rule_set.start, selector_list->start);
        ck_assert_ptr_eq(rule_set.end, declaration_block->end);
        ck_assert_int_eq(rule_set.end->type, token_block_end);
    } else {
        ck_assert_int_eq(rule_set_result, element_error);
    }
}
END_TEST

struct regular_at_rule_test {
    char const* source;
    bool valid;
    size_t length;
    char const* rule_string;
};

static struct regular_at_rule_test regular_at_rule_tests[] = {
    {
        .source = "   \n \t @charset      \"utf-8\";   abc something {}",
        .valid = true,
        .length = strlen("@charset      \"utf-8\""),
        .rule_string = "\"utf-8\"",
    },
    {
        .source = "   \n \t charset      \"utf-8\";   abc something {}", // <-- missing @
        .valid = false,
    },
    {
        .source = "   \n \t @abcdefg      \"utf-8\";   abc something {}", // <-- unknown at-rule
        .valid = false,
    },
};



START_TEST(test_regular_at_rule) {
    struct token* first_token = 0;
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status token_result = parse_tokens(&first_token, &tokens_number, regular_at_rule_tests[_i].source, &lexical_error);
    ck_assert_int_eq(token_result, token_found);

    struct element regular_at_rule;
    struct syntax_error syntax_error;
    enum element_status regular_at_rule_result = parse_at_rule(first_token, &regular_at_rule, &syntax_error);

    if (regular_at_rule_tests[_i].valid) {
        ck_assert_msg(regular_at_rule_result == element_found, "parse error: %s; token: %s", syntax_error.message, syntax_error.token->string);
        ck_assert_int_eq(regular_at_rule.type, element_regular_at_rule);
        ck_assert_int_eq(regular_at_rule.start->type, token_at);
        ck_assert_int_eq(element_count_children(&regular_at_rule), 1);
        ck_assert_int_eq(element_length(&regular_at_rule), regular_at_rule_tests[_i].length);

        struct element* at_rule_rule = regular_at_rule.first_child;
        ck_assert_int_eq(at_rule_rule->type, element_at_rule_rule);
        ck_assert_int_eq(element_length(at_rule_rule), strlen(regular_at_rule_tests[_i].rule_string));

        ck_assert_ptr_eq(regular_at_rule.end, at_rule_rule->end);
    } else {
        ck_assert_int_eq(regular_at_rule_result, element_error);
    }
}
END_TEST

Suite* get_elements_test_suite(void) {
    Suite* suite = suite_create("Elements");

    TCase* common = tcase_create("Common");
    tcase_add_loop_test(common, test_valid_property, 0, sizeof(property_tests) / sizeof(struct property_test));
    tcase_add_loop_test(common, test_valid_value, 0, sizeof(value_tests) / sizeof(struct value_test));
    tcase_add_loop_test(common, test_valid_declaration, 0, sizeof(declaration_tests) / sizeof(struct declaration_test));
    tcase_add_loop_test(common, test_declaration_block, 0, sizeof(declaration_block_tests) / sizeof(struct declaration_block_test));

    TCase* rule_sets = tcase_create("Rule set");
    tcase_add_loop_test(rule_sets, test_selector_list, 0, sizeof(selector_list_tests) / sizeof(struct selector_list_test));
    tcase_add_loop_test(rule_sets, test_rule_set, 0, sizeof(rule_set_tests) / sizeof(struct rule_set_test));

    TCase* at_rules = tcase_create("At rules");
    tcase_add_test(at_rules, test_at_rule_rule);
    tcase_add_loop_test(at_rules, test_regular_at_rule, 0, sizeof(regular_at_rule_tests) / sizeof(struct regular_at_rule_test));

    // suite_add_tcase(suite, rule_sets);
    suite_add_tcase(suite, common);
    suite_add_tcase(suite, rule_sets);
    suite_add_tcase(suite, at_rules);

    return suite;
}
