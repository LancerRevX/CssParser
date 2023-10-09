#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "css/elements.h"

void test_at_rule_rule(void** state) {
    (void)state;

    char const* source = "(max-width: 100px) { /* some rule sets */ }";
    struct token* tokens = calloc(strlen(source), sizeof(struct token));
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status token_result = parse_tokens(tokens, &tokens_number, source, &lexical_error);
    assert_int_equal(token_result, token_found);

    struct element rule;
    struct syntax_error syntax_error;
    enum element_status at_rule_result = parse_at_rule_rule(tokens, &rule, &syntax_error);

    assert_int_equal(at_rule_result, element_found);
    assert_int_equal(element_length(&rule), 18);
    assert_int_equal(rule.start->type, token_parentheses_start);
    assert_int_equal(rule.end->type, token_parentheses_end);
}

void test_valid_property(void** state) {
    (void)state;

    static struct {
        char const* source;
        char const* token_string;
    } tests[] = {
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

    for (size_t _i = 0; _i < sizeof(tests) / sizeof(*tests); _i++) {
        struct token* tokens = calloc(strlen(tests[_i].source), sizeof(struct token));
        size_t tokens_number = 0;
        struct lexical_error lexical_error;
        enum token_status result = parse_tokens(tokens, &tokens_number, tests[_i].source, &lexical_error);
        assert_int_equal(result, token_found);

        struct element property;
        struct syntax_error syntax_error;
        enum element_status property_result = parse_property(tokens, &property, &syntax_error);

        assert_int_equal(property_result, element_found);
        assert_int_equal(property.type, element_property);
        assert_int_equal(element_token_length(&property), 1);
        assert_int_equal(element_length(&property), strlen(tests[_i].token_string));
        assert_string_equal(property.start->string, tests[_i].token_string);
    }
}

void test_valid_value(void** state) {
    (void)state;

    struct {
        char const* source;
        size_t tokens_number;
        size_t element_length;
        char const* start_token_string;
        char const* end_token_string;
    } tests[] = {
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

    for (size_t _i = 0; _i < sizeof(tests) / sizeof(*tests); _i++) {
        struct token* tokens = calloc(strlen(tests[_i].source), sizeof(struct token));
        size_t tokens_number = 0;
        struct lexical_error lexical_error;
        enum token_status result = parse_tokens(tokens, &tokens_number, tests[_i].source, &lexical_error);
        assert_int_equal(result, token_found);

        struct element value;
        struct syntax_error syntax_error;
        enum element_status value_result = parse_value(tokens, &value, &syntax_error);

        assert_int_equal(value_result, element_found);
        assert_int_equal(value.type, element_value);
        assert_int_equal(element_token_length(&value), tests[_i].tokens_number);
        assert_int_equal(element_length(&value), tests[_i].element_length);
        assert_string_equal(value.start->string, tests[_i].start_token_string);
        assert_string_equal(value.end->string, tests[_i].end_token_string);
    }
}

void test_valid_declaration(void** state) {
    (void)state;

    static struct {
        char const* source;
        size_t declaration_length;
        char const* property_string;
        size_t value_length;
    } tests[] = {
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

    for (size_t _i = 0; _i < sizeof(tests) / sizeof(*tests); _i++) {
        struct token* tokens = calloc(strlen(tests[_i].source), sizeof(struct token));
        size_t tokens_number = 0;
        struct lexical_error lexical_error;
        enum token_status token_result = parse_tokens(tokens, &tokens_number, tests[_i].source, &lexical_error);
        assert_int_equal(token_result, token_found);

        struct element declaration;
        struct syntax_error syntax_error;
        enum element_status declaration_result = parse_declaration(tokens, &declaration, &syntax_error);

        if (declaration_result != element_found) {
            fail_msg("parse declaration error: %s; token: %s", syntax_error.message, syntax_error.token->string);
        }

        assert_int_equal(declaration.type, element_declaration);
        assert_int_equal(element_length(&declaration), tests[_i].declaration_length);

        struct element* property = declaration.first_child;
        assert_int_equal(property->type, element_property);
        assert_int_equal(element_length(property), strlen(tests[_i].property_string));
        assert_string_equal(property->start->string, tests[_i].property_string);

        struct element* value = property->next;
        assert_int_equal(value->type, element_value);
        assert_int_equal(element_length(value), tests[_i].value_length);
    }
}

void test_declaration_block(void** state) {
    (void)state;

    static struct {
        char const* source;
        size_t declarations_number;
    } tests[] = {
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

    for (size_t _i = 0; _i < sizeof(tests) / sizeof(*tests); _i++) {
        struct token* tokens = calloc(strlen(tests[_i].source), sizeof(struct token));
        size_t tokens_number = 0;
        struct lexical_error lexical_error;
        enum token_status token_result = parse_tokens(tokens, &tokens_number, tests[_i].source, &lexical_error);
        assert_int_equal(token_result, token_found);

        struct element element;
        struct syntax_error syntax_error;
        enum element_status element_result = parse_declaration_block(tokens, &element, &syntax_error);

        if (element_result != element_found) {
            fail_msg("parse error: %s; token: %s", syntax_error.message, syntax_error.token->string);
        }

        assert_int_equal(element.type, element_declaration_block);

        assert_int_equal(element.start->type, token_block_start);
        assert_int_equal(element.end->type, token_block_end);
        assert_int_equal(element_count_children(&element), tests[_i].declarations_number);
    }
}

void test_selector_list(void** state) {
    (void)state;

    static struct {
        char const* source;
        bool valid;
        size_t selectors_number;
    } tests[] = {
        {
            .source = "abc, selector-2, #some-id > .some-class",
            .valid = true,
            .selectors_number = 3,
        },
    };

    for (size_t _i = 0; _i < sizeof(tests) / sizeof(*tests); _i++) {
        struct token* tokens = calloc(strlen(tests[_i].source), sizeof(struct token));
        size_t tokens_number = 0;
        struct lexical_error lexical_error;
        enum token_status token_result = parse_tokens(tokens, &tokens_number, tests[_i].source, &lexical_error);
        assert_int_equal(token_result, token_found);

        struct element element;
        struct syntax_error syntax_error;
        enum element_status element_result = parse_selector_list(tokens, &element, &syntax_error);

        if (tests[_i].valid) {
            if (element_result != element_found) {
                fail_msg("parse error: %s; token: %s", syntax_error.message, syntax_error.token->string);
            }

            assert_int_equal(element.type, element_selector_list);
            assert_int_equal(element_count_children(&element), tests[_i].selectors_number);
        } else {
            assert_int_equal(element_result, element_error);
        }
    }
}

void test_rule_set(void** state) {
    (void)state;

    struct {
        char const* source;
        bool valid;
        size_t selectors_number;
        size_t declarations_number;
    } tests[] = {
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

    for (size_t _i = 0; _i < sizeof(tests) / sizeof(*tests); _i++) {
        struct token* tokens = calloc(strlen(tests[_i].source), sizeof(struct token));
        size_t tokens_number = 0;
        struct lexical_error lexical_error;
        enum token_status token_result = parse_tokens(tokens, &tokens_number, tests[_i].source, &lexical_error);
        assert_int_equal(token_result, token_found);

        struct element element;
        struct syntax_error syntax_error;
        enum element_status element_result = parse_rule_set(tokens, &element, &syntax_error);

        if (tests[_i].valid) {
            if (element_result != element_found) {
                fail_msg("parse error: %s; token: %s", syntax_error.message, syntax_error.token->string);
            }
            assert_int_equal(element.type, element_rule_set);
            assert_int_equal(element_count_children(&element), 2);

            struct element* selector_list = element.first_child;
            assert_int_equal(selector_list->type, element_selector_list);
            assert_int_equal(element_count_children(selector_list), tests[_i].selectors_number);

            struct element* declaration_block = selector_list->next;
            assert_int_equal(declaration_block->type, element_declaration_block);
            assert_int_equal(element_count_children(declaration_block), tests[_i].declarations_number);

            assert_ptr_equal(element.start, selector_list->start);
            assert_ptr_equal(element.end, declaration_block->end);
            assert_int_equal(element.end->type, token_block_end);
        } else {
            assert_int_equal(element_result, element_error);
        }
    }
}

void test_regular_at_rule(void** state) {
    (void)state;

    static struct {
        char const* source;
        bool valid;
        size_t length;
        char const* rule_string;
    } tests[] = {
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

    for (size_t _i = 0; _i < sizeof(tests) / sizeof(*tests); _i++) {
        struct token* tokens = calloc(strlen(tests[_i].source), sizeof(struct token));
        size_t tokens_number = 0;
        struct lexical_error lexical_error;
        enum token_status token_result = parse_tokens(tokens, &tokens_number, tests[_i].source, &lexical_error);
        assert_int_equal(token_result, token_found);

        struct element element;
        struct syntax_error syntax_error;
        enum element_status element_result = parse_at_rule(tokens, &element, &syntax_error);

        if (tests[_i].valid) {
            if (element_result != element_found) {
                fail_msg("parse error: %s; token: %s", syntax_error.message, syntax_error.token->string);
            }
            assert_int_equal(element.type, element_at_rule);
            assert_int_equal(element.start->type, token_at);
            assert_int_equal(element_count_children(&element), 1);
            assert_int_equal(element_length(&element), tests[_i].length);

            struct element* at_rule_rule = element.first_child;
            assert_int_equal(at_rule_rule->type, element_at_rule_rule);
            assert_int_equal(element_length(at_rule_rule), strlen(tests[_i].rule_string));

            assert_ptr_equal(element.end, at_rule_rule->end);
        } else {
            assert_int_equal(element_result, element_error);
        }
    }
}

void test_nested_at_rule(void** state) {
    (void)state;

    static struct nested_at_rule_test {
        char const* source;
        bool valid;
        char const* rule_string;
        size_t declarations_number;
    } tests[] = {
        {
            .source = "@font-face {"
                      "font-family: 'oskar-md-icons';"
                      "src: url('../fonts/icons.ttf');"
                      "}",
            .valid = true,
            .rule_string = 0,
            .declarations_number = 2,
        },
        {
            .source = "@property --property-name {"
                      "syntax: \"<color>\";"
                      "inherits: false;"
                      "initial-value: #c0ffee;"
                      "}",
            .valid = true,
            .rule_string = "--property-name",
            .declarations_number = 3,
        },
    };

    for (size_t _i = 0; _i < sizeof(tests) / sizeof(struct nested_at_rule_test); _i++) {
        struct token* tokens = calloc(strlen(tests[_i].source), sizeof(struct token));
        size_t tokens_number = 0;
        struct lexical_error lexical_error;
        enum token_status token_result = parse_tokens(tokens, &tokens_number, tests[_i].source, &lexical_error);
        assert_int_equal(token_result, token_found);

        struct element element;
        struct syntax_error syntax_error;
        enum element_status element_result = parse_at_rule(tokens, &element, &syntax_error);

        if (tests[_i].valid) {
            if (element_result != element_found) {
                fail_msg("parse error: %s; token: %s", syntax_error.message, syntax_error.token->string);
            }
            assert_int_equal(element.type, element_at_rule);
            assert_int_equal(element.start->type, token_at);
            assert_int_equal(element.end->type, token_block_end);

            struct element* declaration_block;
            if (tests[_i].rule_string) {
                assert_int_equal(element_count_children(&element), 2);
                struct element* at_rule_rule = element.first_child;
                assert_int_equal(at_rule_rule->type, element_at_rule_rule);
                assert_int_equal(element_length(at_rule_rule), strlen(tests[_i].rule_string));

                declaration_block = at_rule_rule->next;
            } else {
                declaration_block = element.first_child;
            }

            assert_int_equal(declaration_block->start->type, token_block_start);
            assert_int_equal(declaration_block->end->type, token_block_end);
            assert_int_equal(element_count_children(declaration_block), tests[_i].declarations_number);
        } else {
            assert_int_equal(element_result, element_error);
        }
    }
}

void test_media_at_rule(void** state) {
    (void)state;

    static struct media_at_rule_test {
        char const* source;
        bool valid;
        char const* rule_string;
        size_t nested_blocks_number;
    } tests[] = {
        {
            .source = "@media not all and (hover: hover) {"
                      "  abbr::after {"
                      "    content: ' (' attr(title) ')';"
                      "  }"
                      "}",
            .valid = true,
            .rule_string = "not all and (hover: hover)",
            .nested_blocks_number = 1,
        },
        {
            .source = "@media only screen and (min-width: 320px) and (max-width: 480px) and (resolution: 150dpi) {"
                      "  body {"
                      "    line-height: 1.4;"
                      "  }"
                      "}",
            .valid = true,
            .rule_string = "only screen and (min-width: 320px) and (max-width: 480px) and (resolution: 150dpi)",
            .nested_blocks_number = 1,
        },
        {
            .source = "@media (min-width: 577px) and (max-width: 768.98px) {"
                      "  .owl-items-sm-1:not(.owl-loaded) > div {"
                      "    flex: 0 0 100%;"
                      "    width: 100%; }"
                      "  .owl-items-sm-2:not(.owl-loaded) > div {"
                      "    flex: 0 0 50%;"
                      "    width: 50%; }"
                      "  .owl-items-sm-3:not(.owl-loaded) > div {"
                      "    flex: 0 0 33.3333333333%;"
                      "    width: 33.3333333333%; }"
                      "  .owl-items-sm-3 .banner-subtitle,"
                      "  .owl-items-sm-2 .banner-subtitle {"
                      "    font-size: 2vw; }"
                      "  .owl-items-sm-3 .banner-title,"
                      "  .owl-items-sm-2 .banner-title {"
                      "    font-size: 3vw; }"
                      "  .instagram-picture.col-6 {"
                      "    flex: 0 0 33.333333%;"
                      "max-width: 33.333333%; } }",
            .valid = true,
            .rule_string = "(min-width: 577px) and (max-width: 768.98px)",
            .nested_blocks_number = 6,
        },
    };

    for (size_t _i = 0; _i < sizeof(tests) / sizeof(struct media_at_rule_test); _i++) {
        struct token* tokens = calloc(strlen(tests[_i].source), sizeof(struct token));
        size_t tokens_number = 0;
        struct lexical_error lexical_error;
        enum token_status token_result = parse_tokens(tokens, &tokens_number, tests[_i].source, &lexical_error);
        assert_int_equal(token_result, token_found);

        struct element element;
        struct syntax_error syntax_error;
        enum element_status element_result = parse_at_rule(tokens, &element, &syntax_error);

        if (tests[_i].valid) {
            if (element_result != element_found) {
                fail_msg("parse error: %s; token: %s", syntax_error.message, syntax_error.token->string);
            }
            assert_int_equal(element.type, element_at_rule);
            assert_int_equal(element.start->type, token_at);
            assert_int_equal(element.end->type, token_block_end);

            assert_int_equal(element_count_children(&element), 2);
            struct element* at_rule_rule = element.first_child;
            assert_int_equal(at_rule_rule->type, element_at_rule_rule);
            assert_int_equal(element_length(at_rule_rule), strlen(tests[_i].rule_string));

            struct element* declaration_block = at_rule_rule->next;
            assert_int_equal(declaration_block->start->type, token_block_start);
            assert_int_equal(declaration_block->end->type, token_block_end);
            assert_int_equal(element_count_children(declaration_block), tests[_i].nested_blocks_number);
        } else {
            assert_int_equal(element_result, element_error);
        }
    }
}

void test_real_css(void** state) {
    (void)state;

    char const* source = "/*"
                         " Theme Name:   Woodmart Child"
                         " Description:  Woodmart Child Theme"
                         " Author:       XTemos"
                         " Author URI:   http://xtemos.com"
                         " Template:     woodmart"
                         " Version:      1.0.0"
                         " Text Domain:  woodmart"
                         "*/"
                         ""
                         "@font-face {"
                         "    font-family: 'oskar-md-icons';"
                         "    src: url('../fonts/icons.ttf');"
                         "}"
                         ""
                         ".i-mattress {"
                         ""
                         "    font-family: 'oskar-md-icons';"
                         "    font-style: normal;"
                         "    font-weight: 400;"
                         "    font-variant: normal;"
                         ""
                         "}"
                         ""
                         ".i-mattress::before {"
                         "    font-family: 'oskar-md-icons';"
                         "    content: \"\\e958\";"
                         "}"
                         ""
                         "/* подпись размера для одеял и подушек */"
                         ".my-size-annotation {"
                         "    color: var(--wd-text-color);"
                         "    font-size: 95%;"
                         "    font-weight: 400;"
                         "}"
                         ""
                         "/* убрать артикул на странице товара */"
                         "span.sku_wrapper {"
                         "	display: none;"
                         "}"
                         ""
                         ""
                         ".wd-pf-btn {"
                         "	width: 100% !important;"
                         "}"
                         ""
                         ".wd-pf-btn button {"
                         "	width: 100%;"
                         "}"
                         ""
                         ":root {"
                         "    --my-color-vk: #0077ff;"
                         "    --my-color-white: #ffffff;"
                         "    --my-color-black: #2b2a29;"
                         "    --my-color-light-gray: rgb(245, 245, 245);"
                         "    --my-color-gray-background: rgb(248, 248, 248);"
                         "    --my-color-gray-border: rgb(216, 216, 216);"
                         "    --my-color-red: #e31e24;"
                         "    --my-color-green: #9eff00;"
                         "    --my-color-green-dark: rgb(21, 156, 0);"
                         "    --my-color-gray: rgb(102, 102, 102);"
                         "    --my-color-dark-gray: rgb(84, 84, 84);"
                         "    --my-color-vk: #0077FF;"
                         "    --my-color-whatsapp: #25D366;"
                         "    --my-color-viber: #7360f2;"
                         "    --my-color-telegram: #2AABEE;"
                         ""
                         "    --my-main-width: 1222px;"
                         "    --my-main-padding: 15px;"
                         ""
                         "    --my-text-font-size: 14px;"
                         "    --my-title-font-size: 20px;"
                         "}"
                         ""
                         ".my-main {"
                         "    max-width: var(--my-main-width);"
                         "    margin: 0 auto;"
                         "    padding: 0 var(--my-main-padding);"
                         "}"
                         ""
                         ".my-button {"
                         "    display: flex;"
                         "    flex-direction: row;"
                         "    justify-content: center;"
                         "    align-items: center;"
                         "    gap: 0.4em;"
                         "    max-width: 100%;"
                         "    padding: 14px 20px;"
                         "    box-sizing: border-box;"
                         "    text-transform: uppercase;"
                         "    background-color: #f3f3f3;"
                         "    color: #3e3e3e; "
                         "    font-size: 13px;"
                         "    font-weight: 600;"
                         "    line-height: 18px;"
                         "    text-align: center;"
                         "}"
                         ""
                         ".my-button:hover {"
                         "    box-shadow: inset 0 0 200px rgba(0,0,0,0.1);"
                         "    color: #3e3e3e;"
                         "}"
                         ""
                         ".my-button.red {"
                         "    background-color: var(--my-color-red);"
                         "    color: var(--my-color-white);"
                         "}"
                         ""
                         ".my-button.vk {"
                         "    background-color: var(--my-color-vk);"
                         "    color: var(--my-color-white);"
                         "}"
                         ""
                         ".my-button-icon {"
                         "    font-size: 18px;"
                         "}"
                         ""
                         ""
                         ".checkout .optional {"
                         "    display: none;"
                         "}"
                         ""
                         "#billing_country_field {"
                         "    display: none;"
                         "}"
                         ""
                         "/* Кнопка \"В корзину\" на странице товара */"
                         ""
                         "form.cart {"
                         "    display: flex;"
                         "    flex-direction: row;"
                         "}"
                         ""
                         ".single_add_to_cart_button {"
                         "    flex: 1;"
                         "}"
                         ""
                         "form.variations_form.cart {"
                         "    flex-direction: column;"
                         "}"
                         ""
                         "form.variations_form.cart select {"
                         "    width: 100%;"
                         "    max-width: 100%;"
                         "}"
                         ""
                         ".woocommerce-variation-add-to-cart  {"
                         "    display: flex;"
                         "    flex-direction: row;"
                         "}"
                         ""
                         ""
                         ""
                         ""
                         ""
                         ".category-image :nth-child(2) {"
                         "    display: none;"
                         "} /* фикс бага с изображением категории */"
                         ""
                         ".entry-meta-list {"
                         "	display: none;"
                         "} /* имя автора статьи */"
                         ""
                         ".dd-selected-image, .dd-option-image {"
                         "	display: none;"
                         "} /* картинка для опции \"нет\" составного товара */"
                         ""
                         ".cart_item .woocommerce-placeholder {"
                         "	display: none;"
                         "} "
                         ".wooco-cart-item .woocommerce-placeholder {"
                         "	opacity: 0;"
                         "}/* картинка компонента в корзине */"
                         ""
                         "/* порядок кнопок социальных сетей */"
                         ".wd-social-icons {"
                         "	display: flex;"
                         "	justify-content: center;"
                         "}"
                         ".social-vk {"
                         "	order: 1;"
                         "	margin-left: 0;"
                         "}"
                         ".social-ok {"
                         "	order: 2;"
                         "}"
                         ".social-tg {"
                         "	order: 3;"
                         "}"
                         ".social-youtube {"
                         "	order: 4;"
                         "}"
                         ""
                         "/*убрать дату с статей*/"
                         ".post-date {"
                         "	display: none !important;"
                         "}"
                         ""
                         "/*удаление огромного отступа у заголовка страницы*/"
                         "/*.page-title {"
                         "	margin-bottom: 0;"
                         "}"
                         "*/";
    struct token* tokens = calloc(strlen(source), sizeof(struct token));
    size_t tokens_number = 0;
    struct lexical_error lexical_error;
    enum token_status token_result = parse_tokens(tokens, &tokens_number, source, &lexical_error);
    assert_int_equal(token_result, token_found);

    struct element* first_element;
    struct syntax_error syntax_error;
    enum element_status result = parse_elements(tokens, &first_element, &syntax_error);

    assert_int_equal(result, element_found);
}

static const struct CMUnitTest element_tests[] = {
    cmocka_unit_test(test_at_rule_rule),
    cmocka_unit_test(test_valid_property),
    cmocka_unit_test(test_valid_value),
    cmocka_unit_test(test_valid_declaration),
    cmocka_unit_test(test_declaration_block),
    cmocka_unit_test(test_selector_list),
    cmocka_unit_test(test_rule_set),
    cmocka_unit_test(test_regular_at_rule),
    cmocka_unit_test(test_nested_at_rule),
    cmocka_unit_test(test_media_at_rule),
    cmocka_unit_test(test_real_css),
};

int main(void) {
    cmocka_run_group_tests(element_tests, NULL, NULL);
}
