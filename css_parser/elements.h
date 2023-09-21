#pragma once

#include "tokens.h"

enum element_status {
    element_found,
    element_not_found,
};

enum element_type {
    element_regular_at_rule,
    element_nested_at_rule,
    element_conditional_group_at_rule,
    element_rule_set,
    element_declaration,
    element_property,
    element_value,
};

enum element_complexity {
    element_simple,
    element_complex,
};

const enum element_complexity element_complexity[] = {
    [element_rule_set] = element_complex,
    [element_declaration] = element_complex,
    [element_property] = element_simple,
    [element_value] = element_simple,
};

struct element {
    enum element_type type;
    union {
        struct token* token;
        struct element* child_elements;
    };
};

const char* regular_at_rules[] = {
    "charset",
    "import",
    "namespace",
    0
};

const char* nested_at_rules[] = {
    "page",
    "font-face",
    "keyframes",
    "counter-style",
    "font-feature-values",
    "property",
    "layer",
    0
};

const char* conditional_group_at_rules[] = {
    "media",
    "supports",
    "document",
    0
};

enum element_status parse_elements(struct token* tokens, size_t tokens_number,
                                   struct element* elements, size_t* elements_number);



enum element_status parse_elements(struct token* tokens, size_t tokens_number,
                                   struct element* elements, size_t* elements_number) {
    for (size_t i = 0; i < tokens_number; i++) {
        struct token* token = &tokens[i];

    }
}

enum element_status get_regular_at_rule(struct token* token, struct element* element) {
    if (token->length < 2 || token->pointer[0] != '@') {
        return element_not_found;
    }

    bool valid_regular_at_rule = false;
    for (const char** regular_at_rule; regular_at_rule != 0; regular_at_rule++) {
        if (strncmp(token->pointer, *regular_at_rule, strlen(*regular_at_rule)) == 0) {
            valid_regular_at_rule = true;
        }
    }
    if (!valid_regular_at_rule) {
        return element_not_found;
    }
}

enum element_status get_at_rule(struct token* token, struct element* element) {
    if (token->length < 2 || token->pointer[0] != '@') {
        return element_not_found;
    }

    
}