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
    element_rule_set,
    element_selector,
    element_declaration,
    element_property,
    element_value,
};

char const* const element_name[] = {
    [element_regular_at_rule] = "Regular at rule",
    [element_nested_at_rule] = "Nested at rule",
    [element_conditional_group_at_rule] = "Conditional group at rule",
    [element_rule_set] = "Rule set",
    [element_declaration] = "Declaration",
    [element_property] = "Property",
    [element_value] = "Value",
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
    return (element->end->pointer - element->start->pointer) + element->end->length;
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

element_status get_rule_set(struct token** token, struct element* element, struct syntax_error* error) {
    if ((*token)->type != token_at) {
        error->message = "expected '@' at the start of at-rule";
        error->token = *token;
        return element_error;
    }
    element->start = *token;

    (*token) = (*token)->next;

    if ((*token)->type != token_identifier) {
        error->message = "expected at-rule identifier";
        error->token = *token;
        return element_error;
    }
    int type = get_at_rule_type((*token)->pointer);

    (*token) = (*token)->next;
}

element_status get_regular_at_rule(struct token* token, struct element* element) {
    // if (token->length < 2 || token->pointer[0] != '@') {
    //     return element_not_found;
    // }

    // bool valid_regular_at_rule = false;
    // for (const char** regular_at_rule; regular_at_rule != 0; regular_at_rule++) {
    //     if (strncmp(token->pointer, *regular_at_rule, strlen(*regular_at_rule)) == 0) {
    //         valid_regular_at_rule = true;
    //     }
    // }
    // if (!valid_regular_at_rule) {
    //     return element_not_found;
    // }
}

int get_at_rule_type(char const* identifier) {
}

element_status get_at_rule(struct token** token, struct element* element, struct syntax_error* error) {
    if ((*token)->type != token_at) {
        error->message = "expected '@' at the start of at-rule";
        error->token = *token;
        return element_error;
    }
    element->start = *token;

    (*token) = (*token)->next;

    if ((*token)->type != token_identifier) {
        error->message = "expected at-rule identifier";
        error->token = *token;
        return element_error;
    }
    int type = get_at_rule_type((*token)->pointer);

    (*token) = (*token)->next;
}

element_status parse_elements(struct token* first_token, struct element** first_element, size_t* elements_number) {
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
            get_at_rule(&token, &element);
            break;
        }
    }
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
