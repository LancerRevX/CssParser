#pragma once

#include "tokens.h"

typedef enum element_status {
    element_found,
    element_error,
} element_status;

enum element_type {
    element_regular_at_rule,
    element_nested_at_rule,
    element_conditional_group_at_rule,
    element_at_rule_rule,
    element_rule_set,
    element_selector,
    element_declaration,
    element_property,
    element_value,

    element_types_number
};

static char const* const element_name[] = {
    [element_regular_at_rule] = "Regular at rule",
    [element_nested_at_rule] = "Nested at rule",
    [element_conditional_group_at_rule] = "Conditional group at rule",
    [element_at_rule_rule] = "Rule of at-rule",
    [element_rule_set] = "Rule set",
    [element_selector] = "Selector",
    [element_declaration] = "Declaration",
    [element_property] = "Property",
    [element_value] = "Value",
};

struct at_rule {
    struct token* identifier;
    struct element* rule;
    struct element* block;
};

struct declaration {
    struct element* property;
    struct element* value;
};

struct rule_set {
    struct element* selectors;
    struct element* declarations;
};

struct element {
    enum element_type type;

    struct token* start;
    struct token* end;
    struct element* first_child;

    union {
        struct at_rule at_rule;
        struct rule_set rule_set;
        struct declaration declaration;
    };

    struct element* next;
};

struct syntax_error {
    char const* message;
    struct token* token;
};

const char* const regular_at_rules[] = {
    "charset",
    "import",
    "namespace",
    0};

const char* const nested_at_rules[] = {
    "page",
    "font-face",
    "keyframes",
    "counter-style",
    "font-feature-values",
    "property",
    "layer",
    0};

const char* const conditional_group_at_rules[] = {
    "media",
    "supports",
    "document",
    0};

void free_elements(struct element* first_element) {
    for (struct element* element = first_element; element != 0;) {
        struct element* previous = element;
        element = element->next;
        free(previous);
    }
}

size_t element_length(struct element* element) {
    return ((size_t)element->end->pointer - (size_t)element->start->pointer) + element->end->length;
}

size_t element_add_child(struct element* element, struct element* new_child) {
    if (element->first_child == 0) {
        element->first_child = new_child;
        return 1;
    } else {
        size_t i = 1;
        struct element* child = element->first_child;
        while (child->next != 0) {
            child = child->next;
            i++;
        }
        child->next = new_child;
        return i + 1;
    }
}

void print_element_content(struct element* element) {
    printf("%.*s", (int)element_length(element), element->start->pointer);
}

void print_declaration(struct element* element) {
    print_element_content(element->declaration.property);
    printf(": ");
    print_element_content(element->declaration.value);
    printf(";\n");
}

void print_element_tree(struct element* element, size_t depth) {
    for (size_t i = 0; i < depth; i++) {
        printf("\t");
    }

    if (element->type == element_declaration) {
        print_declaration(element);
        return;
    }

    if (element->first_child) {
        printf("%s:\n", element_name[element->type]);
        for (struct element* child = element->first_child; child; child = child->next) {
            print_element_tree(child, depth + 1);
        }
    } else {
        printf("%s: \"", element_name[element->type]);
        print_element_content(element);
        printf("\"\n");
    }
}

enum element_type get_at_rule_type(char const* identifier) {
    for (char const* const* at_rule = regular_at_rules; *at_rule; at_rule++) {
        if (strcmp(*at_rule, identifier) == 0) {
            return element_regular_at_rule;
        }
    }
    for (char const* const* at_rule = nested_at_rules; *at_rule; at_rule++) {
        if (strcmp(*at_rule, identifier) == 0) {
            return element_nested_at_rule;
        }
    }
    for (char const* const* at_rule = conditional_group_at_rules; *at_rule; at_rule++) {
        if (strcmp(*at_rule, identifier) == 0) {
            return element_conditional_group_at_rule;
        }
    }
    return -1;
}

element_status get_rule_set(struct token** token, struct element* element, struct syntax_error* error) {
    return element_found;
}

element_status parse_at_rule_rule(struct token** token, struct element* rule, struct syntax_error* error) {
    rule->type = element_at_rule_rule;
    rule->first_child = 0;
    rule->next = 0;

    struct token* const first_token = *token;
    struct token* last_meaning_token = 0;
    struct token* last_parenthesis = 0;
    size_t tokens_number = 0;
    size_t parentheses = 0;
    while (*token) {
        if ((*token)->type == token_block_start) {
            break;
        }

        if ((*token)->type == token_semicolon && parentheses == 0) {
            break;
        }

        if ((*token)->type == token_space || (*token)->type == token_comment) {
            *token = (*token)->next;
            continue;
        }

        if ((*token)->type == token_parentheses_start) {
            parentheses++;
            last_parenthesis = *token;
        } else if ((*token)->type == token_parentheses_end) {
            if (parentheses > 0) {
                parentheses--;
            } else {
                error->message = "unmatched closing parenthesis";
                error->token = *token;
                return element_error;
            }
        }

        if (tokens_number == 0) {
            rule->start = *token;
        }

        tokens_number++;
        last_meaning_token = *token;
        *token = (*token)->next;
    }

    if (tokens_number == 0) {
        error->message = "empty at-rule rule";
        error->token = first_token;
        return element_error;
    }

    if (parentheses > 0) {
        error->message = "unclosed parentheses";
        error->token = last_parenthesis;
        return element_error;
    }

    rule->end = last_meaning_token;
    return element_found;
}

element_status get_regular_at_rule_body(struct token** token, struct element* element, struct syntax_error* error) {
}

element_status get_nested_at_rule(struct token** token, struct element* element, struct syntax_error* error) {
}

element_status get_conditional_group_at_rule(struct token** token, struct element* element, struct syntax_error* error) {
}

element_status parse_at_rule(struct token** token, struct element* at_rule, struct syntax_error* error) {
    at_rule->start = *token;
    at_rule->first_child = 0;
    at_rule->next = 0;

    (*token) = (*token)->next;
    if (!(*token) || (*token)->type != token_identifier) {
        error->message = "expected at-rule identifier";
        error->token = *token;
        return element_error;
    }

    int type = get_at_rule_type((*token)->pointer);
    if (type == -1) {
        error->message = "unknown at-rule identifier";
        error->token = *token;
        return element_error;
    }

    (*token) = (*token)->next;

    struct element* rule = malloc(sizeof(struct element));
    enum element_status result;
    result = parse_at_rule_rule(token, rule, error);
    if (result != element_found) {
        free(rule);
        return result;
    }
    at_rule->at_rule.rule = rule;

    if (type == element_regular_at_rule) {
        return element_found;
    }

    switch (type) {
    case element_regular_at_rule:
        return element_found;
    case element_nested_at_rule:
        result = parse_declaration_block(token, block, error);
        break;
    case element_conditional_group_at_rule:
        result = parse_conditional_group_block(token, block, error);
        break;
    }

    return result;
}

element_status parse_elements(struct token* first_token, struct element** first_element, size_t* elements_number, struct syntax_error* error) {
    struct element* element = 0;
    *elements_number = 0;

    struct token* token = first_token;
    while (token) {
        if (element == 0) {
            element = malloc(sizeof(struct element));
            *first_element = element;
        } else {
            element->next = malloc(sizeof(struct element));
            element = element->next;
        }
        element->next = 0;

        switch (token->type) {
        case token_space:
        case token_comment:
            token = token->next;
            continue;
        case token_at:
            parse_at_rule(&token, element, error);
            break;
        case token_dot:
        case token_hash:
        case token_identifier:
            get_rule_set(&token, element, error);
            break;
        default:
            error->message = "unexpected token";
            error->token = token;
            free_elements(*first_element);
            return element_error;
        }
    }

    return element_found;
}

size_t count_elements_of_type(struct element* element, enum element_type type) {
    size_t number_of_type = 0;
    for (struct element* child = element->first_child; child; child = child->next) {
        number_of_type += count_elements_of_type(element, type);
    }
    return number_of_type;
}

void print_elements_of_type(struct element* element, enum element_type type) {
    for (struct element* child = element->first_child; child; child = child->next) {
        if (child->type == type) {
            print_element_tree(child, 0);
        } else {
            print_elements_of_type(element, type);
        }
    }
}

void print_vars(struct element* element) {
    if (element->type == element_declaration) {
        char const* property_source = element->declaration.property->start->pointer;
        if (strncmp(property_source, "--", 2) == 0) {
            print_declaration(element);
        }
        return;
    }

    for (struct element* child = element->first_child; child; child = child->next) {
        print_vars(child);
    }
}
