#pragma once

#include "tokens.h"

typedef enum element_status {
    element_found,
    element_error,
} element_status;

enum element_type {
    element_at_rule,
    element_at_rule_rule,
    element_rule_set,
    element_selector,
    element_selector_list,
    element_declaration,
    element_declaration_block,
    element_property,
    element_value,

    element_types_number
};

static char const* const element_name[] = {
    [element_at_rule] = "At rule",
    [element_at_rule_rule] = "Rule of at-rule",
    [element_rule_set] = "Rule set",
    [element_selector] = "Selector",
    [element_selector_list] = "Selector list",
    [element_declaration] = "Declaration",
    [element_declaration_block] = "Declaration block",
    [element_property] = "Property",
    [element_value] = "Value",
};

struct element {
    enum element_type type;

    struct token* start;
    struct token* end;
    struct element* first_child;

    struct element* next;
};

struct syntax_error {
    char const* message;
    struct token* token;
};

struct at_rule_syntax {
    char const* identifier;
    bool rule_allowed;
    bool rule_required;
    bool block_allowed;
    bool block_required;
    bool block_nested;
};

struct at_rule_syntax at_rule_syntaxes[] = {
    {
        .identifier = "charset",
        .rule_required = true,
        .block_allowed = false,
    },
    {
        .identifier = "import",
        .rule_required = true,
        .block_allowed = false,
    },
    {
        .identifier = "namespace",
        .rule_required = true,
        .block_allowed = false,
    },
    {
        .identifier = "font-face",
        .rule_allowed = false,
        .rule_required = false,
        .block_required = true,
        .block_nested = false,
    },
    {
        .identifier = "property",
        .rule_required = true,
        .block_required = true,
        .block_nested = false,
    },
    {
        .identifier = "keyframes",
        .rule_required = true,
        .block_required = true,
        .block_nested = true,
    },
    {
        .identifier = "media",
        .rule_required = true,
        .block_required = true,
        .block_nested = true,
    },
};

void element_free(struct element* element) {
    if (element == 0) {
        return;
    }

    if (element->first_child) {
        element_free(element->first_child);
    }
    if (element->next) {
        element_free(element->next);
    }
    free(element);
}

size_t element_length(struct element* element) {
    return ((size_t)element->end->pointer - (size_t)element->start->pointer) + element->end->length;
}

size_t element_token_length(struct element* element) {
    size_t token_length = 1;
    for (struct token* token = element->start; token != element->end; token = token->next) {
        token_length++;
    }
    return token_length;
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

void element_init(struct element* element, enum element_type type) {
    element->type = type;
    element->start = 0;
    element->end = 0;
    element->first_child = 0;
    element->next = 0;
}

void element_print_content(struct element* element) {
    printf("%.*s", (int)element_length(element), element->start->pointer);
}

void declaration_print(struct element* declaration) {
    struct element* property = declaration->first_child;
    struct element* value = property->next;

    element_print_content(property);
    printf(": ");
    element_print_content(value);
    printf(";\n");
}

size_t element_count_children(struct element* element) {
    size_t result = 0;
    for (struct element* child = element->first_child; child != 0; child = child->next) {
        result++;
    }
    return result;
}

void element_print_tree(struct element* element, size_t depth) {
    for (size_t i = 0; i < depth; i++) {
        printf("\t");
    }

    if (element->type == element_declaration) {
        declaration_print(element);
        return;
    }

    if (element->first_child) {
        printf("%s:\n", element_name[element->type]);
        for (struct element* child = element->first_child; child; child = child->next) {
            element_print_tree(child, depth + 1);
        }
    } else {
        printf("%s: \"", element_name[element->type]);
        element_print_content(element);
        printf("\"\n");
    }
}

void skip_spaces_and_comments(struct token** token_ptr) {
    while (*token_ptr) {
        switch ((*token_ptr)->type) {
        case token_space:
        case token_comment:
            *token_ptr = (*token_ptr)->next;
            break;
        default:
            return;
        }
    }
}

element_status parse_property(struct token* first_token, struct element* property, struct syntax_error* error) {
    struct token* token = first_token;

    while (token) {
        switch (token->type) {
        case token_space:
        case token_comment:
            token = token->next;
            break;
        case token_identifier:
            property->type = element_property;
            property->start = token;
            property->end = token;
            property->first_child = 0;
            property->next = 0;
            return element_found;
        default:
            error->message = "unexpected token while parsing property";
            error->token = token;
            return element_error;
        }
    }

    error->message = "expected property";
    error->token = first_token;
    return element_error;
}

element_status parse_value(struct token* first_token, struct element* value, struct syntax_error* error) {
    struct token* token = first_token;

    value->type = element_value;
    value->first_child = 0;
    value->next = 0;

    struct token* last_meaning_token = 0;
    struct token* last_parenthesis = 0;
    size_t tokens_number = 0;
    size_t parentheses = 0;
    while (1) {
        if (!token || token->type == token_block_end) {
            break;
        }

        if (token->type == token_semicolon && parentheses == 0) {
            break;
        }

        switch (token->type) {
        case token_space:
        case token_comment:
            token = token->next;
            continue;
        case token_identifier:
        case token_number:
        case token_comma:
        case token_string:
        case token_percent:
        case token_hash:
        case token_exclamation:
            break;
        case token_parentheses_start:
            parentheses++;
            last_parenthesis = token;
            break;
        case token_parentheses_end:
            if (parentheses == 0) {
                error->message = "unmatched closing parenthesis";
                error->token = token;
                return element_error;
            }
            parentheses--;
            break;
        default:
            if (parentheses == 0) {
                error->message = "unexpected token while parsing value";
                error->token = token;
                return element_error;
            }
            break;
        }

        if (tokens_number == 0) {
            value->start = token;
        }
        tokens_number++;
        last_meaning_token = token;
        token = token->next;
    }

    if (tokens_number == 0) {
        error->message = "empty value";
        error->token = first_token;
        return element_error;
    }

    if (parentheses > 0) {
        error->message = "unclosed parentheses";
        error->token = last_parenthesis;
        return element_error;
    }

    value->end = last_meaning_token;
    return element_found;
}

element_status parse_declaration(struct token* first_token, struct element* declaration, struct syntax_error* error) {
    struct token* token = first_token;

    element_init(declaration, element_declaration);

    struct element* property = malloc(sizeof(struct element));
    enum element_status result = parse_property(token, property, error);
    if (result != element_found) {
        free(property);
        return result;
    }

    element_add_child(declaration, property);
    declaration->start = property->start;

    token = property->end->next;

    while (1) {
        if (!token) {
            error->message = "expected colon (\":\") after property";
            error->token = declaration->end;
            return element_error;
        } else if (token->type == token_colon) {
            break;
        } else if (token->type == token_space || token->type == token_comment) {
            token = token->next;
        } else {
            error->message = "unexpected token while parsing declaration";
            error->token = token;
            return element_error;
        }
    }
    token = token->next;

    struct element* value = malloc(sizeof(struct element));
    result = parse_value(token, value, error);
    if (result != element_found) {
        free(value);
        element_free(declaration->first_child);
        return result;
    }
    element_add_child(declaration, value);
    declaration->end = value->end;
    return element_found;
}

element_status parse_declaration_block(struct token* first_token, struct element* block, struct syntax_error* error) {
    struct token* token = first_token;

    element_init(block, element_declaration_block);

    skip_spaces_and_comments(&token);

    if (!token || token->type != token_block_start) {
        error->message = "declaration block must start with \"{\"";
        error->token = token;
        return element_error;
    }

    block->start = token;

    token = token->next;

    skip_spaces_and_comments(&token);

    while (true) {
        if (!token) {
            error->message = "missing \"}\" at the end of declaration block";
            error->token = first_token;
            return element_error;
        }

        struct element* declaration = malloc(sizeof(struct element));
        element_status result = parse_declaration(token, declaration, error);
        if (result != element_found) {
            element_free(block->first_child);
            free(declaration);
            return result;
        }
        element_add_child(block, declaration);
        token = declaration->end->next;

        skip_spaces_and_comments(&token);

        if (!token) {
            error->message = "missing \"}\" at the end of declaration block";
            error->token = first_token;
            return element_error;
        }

        if (token->type == token_semicolon) {
            token = token->next;
            skip_spaces_and_comments(&token);
        }

        if (!token) {
            error->message = "missing \"}\" at the end of declaration block";
            error->token = first_token;
            return element_error;
        }

        if (token->type == token_block_end) {
            block->end = token;
            return element_found;
        }
    }
}

element_status parse_selector(struct token* first_token, struct element* selector, struct syntax_error* error) {
    struct token* token = first_token;

    element_init(selector, element_selector);

    skip_spaces_and_comments(&token);

    selector->start = token;
    struct token* bracket = 0;
    size_t tokens_number = 0;
    while (1) {
        if (!token || token->type == token_block_start || token->type == token_comma) {
            if (tokens_number == 0) {
                error->message = "empty selector";
                error->token = first_token;
                return element_error;
            } else if (bracket) {
                error->message = "unmatched bracket";
                error->token = bracket;
                return element_error;
            } else {
                return element_found;
            }
        }

        switch (token->type) {
        case token_bracket_start:
        case token_parentheses_start:
            if (bracket) {
                error->message = "nested paretheses";
                error->token = token;
                return element_error;
            }
            bracket = token;
            break;
        case token_bracket_end:
        case token_parentheses_end:
            if (!bracket) {
                error->message = "unmatched bracket";
                error->token = token;
                return element_error;
            }
            if ((token->type == token_bracket_end && bracket->type != token_bracket_start) ||
                (token->type == token_parentheses_end && bracket->type != token_parentheses_start)) {
                error->message = "invalid closing bracket";
                error->token = token;
                return element_error;
            }
            bracket = 0;
            break;
        case token_hash:
        case token_dot:
        case token_colon:
        case token_greater_than:
        case token_identifier:
        case token_string:
            break;
        case token_space:
        case token_comment:
            token = token->next;
            continue;
        default:
            if (!bracket) {
                error->message = "unexpected token while parsing selector";
                error->token = token;
                return element_error;
            } else {
                break;
            }
        }

        selector->end = token;
        token = token->next;
        tokens_number++;
    }
}

element_status parse_selector_list(struct token* first_token, struct element* selector_list, struct syntax_error* error) {
    struct token* token = first_token;

    element_init(selector_list, element_selector_list);

    skip_spaces_and_comments(&token);

    size_t selectors_number = 0;
    selector_list->start = token;

    while (token && token->type != token_block_start) {
        if (selectors_number > 0) {
            if (token->type != token_comma) {
                error->message = "expected comma";
                error->token = token;
                return element_error;
            }
            token = token->next;
        }

        struct element* selector = malloc(sizeof(struct element));
        element_status selector_result = parse_selector(token, selector, error);
        if (selector_result != element_found) {
            element_free(selector_list->first_child);
            free(selector);
            return selector_result;
        }
        element_add_child(selector_list, selector);
        selectors_number++;
        selector_list->end = selector->end;

        token = selector->end->next;
        skip_spaces_and_comments(&token);
    }

    if (selectors_number == 0) {
        error->message = "empty selector list";
        error->token = first_token;
        return element_error;
    }

    return element_found;
}

element_status parse_rule_set(struct token* first_token, struct element* rule_set, struct syntax_error* error) {
    struct token* token = first_token;

    element_init(rule_set, element_rule_set);

    skip_spaces_and_comments(&token);

    struct element* selector_list = malloc(sizeof(struct element));
    element_status selector_list_result = parse_selector_list(token, selector_list, error);
    if (selector_list_result != element_found) {
        free(selector_list);
        return selector_list_result;
    }
    element_add_child(rule_set, selector_list);
    rule_set->start = selector_list->start;

    token = selector_list->end->next;
    struct element* declaration_block = malloc(sizeof(struct element));
    element_status declaration_block_result = parse_declaration_block(token, declaration_block, error);
    if (declaration_block_result != element_found) {
        free(selector_list);
        free(declaration_block);
        return declaration_block_result;
    }
    element_add_child(rule_set, declaration_block);
    rule_set->end = declaration_block->end;
    selector_list->next = declaration_block;

    return element_found;
}

element_status parse_at_rule_rule(struct token* first_token, struct element* rule, struct syntax_error* error) {
    struct token* token = first_token;

    rule->type = element_at_rule_rule;
    rule->first_child = 0;
    rule->next = 0;

    struct token* last_meaning_token = 0;
    struct token* last_parenthesis = 0;
    size_t tokens_number = 0;
    size_t parentheses = 0;
    while (token) {
        if (token->type == token_block_start) {
            break;
        }

        if (token->type == token_semicolon && parentheses == 0) {
            break;
        }

        if (token->type == token_space || token->type == token_comment) {
            token = token->next;
            continue;
        }

        if (token->type == token_parentheses_start) {
            parentheses++;
            last_parenthesis = token;
        } else if (token->type == token_parentheses_end) {
            if (parentheses > 0) {
                parentheses--;
            } else {
                error->message = "unmatched closing parenthesis";
                error->token = token;
                return element_error;
            }
        }

        if (tokens_number == 0) {
            rule->start = token;
        }

        tokens_number++;
        last_meaning_token = token;
        token = token->next;
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

element_status parse_at_rule(struct token* first_token, struct element* at_rule, struct syntax_error* error);

element_status parse_conditional_group_block(struct token* first_token, struct element* block, struct syntax_error* error) {
    struct token* token = first_token;

    element_init(block, element_declaration_block);

    skip_spaces_and_comments(&token);

    if (token->type != token_block_start) {
        error->message = "block must start with \"{\"";
        error->token = token;
        return element_error;
    }

    block->start = token;

    token = token->next;

    while (1) {
        skip_spaces_and_comments(&token);

        if (!token) {
            error->message = "block must end with \"}\"";
            error->token = first_token;
            return element_error;
        }

        if (token->type == token_block_end) {
            block->end = token;
            return element_found;
        }

        struct element* new_chlid = malloc(sizeof(struct element));
        element_status new_child_result;
        if (token->type == token_at) {
            new_child_result = parse_at_rule(token, new_chlid, error);
        } else {
            new_child_result = parse_rule_set(token, new_chlid, error);
        }
        if (new_child_result != element_found) {
            element_free(block->first_child);
            free(new_chlid);
            return new_child_result;
        }
        element_add_child(block, new_chlid);
        block->end = new_chlid->end;
        token = block->end->next;
    }
}

element_status parse_at_rule(struct token* first_token, struct element* at_rule, struct syntax_error* error) {
    struct token* token = first_token;

    skip_spaces_and_comments(&token);

    if (!token || token->type != token_at) {
        error->message = "at rule must start with \"@\"";
        error->token = token;
        return element_error;
    }

    element_init(at_rule, element_at_rule);
    at_rule->start = token;

    token = token->next;

    if (!token || token->type != token_identifier) {
        error->message = "expected at-rule identifier";
        error->token = token;
        return element_error;
    }

    struct at_rule_syntax* syntax = 0;
    for (size_t i = 0; i < sizeof(at_rule_syntaxes) / sizeof(struct at_rule_syntax); i++) {
        if (strcmp(at_rule_syntaxes[i].identifier, token->string) == 0) {
            syntax = &at_rule_syntaxes[i];
            break;
        }
    }
    if (!syntax) {
        error->message = "unknown at-rule identifier";
        error->token = token;
        return element_error;
    }

    token = token->next;

    skip_spaces_and_comments(&token);

    if (syntax->rule_required || (syntax->rule_allowed && token->type != token_block_start)) {
        struct element* rule = malloc(sizeof(struct element));
        enum element_status result = parse_at_rule_rule(token, rule, error);
        if (result != element_found) {
            free(rule);
            return result;
        }
        element_add_child(at_rule, rule);

        at_rule->end = rule->end;
        token = rule->end->next;
    }

    skip_spaces_and_comments(&token);

    if (syntax->block_required || (syntax->block_allowed && token->type == token_block_start)) {
        enum element_status block_result;
        struct element* block = malloc(sizeof(struct element));
        if (syntax->block_nested) {
            block_result = parse_conditional_group_block(token, block, error);
        } else {
            block_result = parse_declaration_block(token, block, error);
        }

        if (block_result != element_found) {
            element_free(at_rule->first_child);
            free(block);
            return block_result;
        }

        element_add_child(at_rule, block);
        at_rule->end = block->end;
    }

    return element_found;
}

element_status parse_elements(struct token* first_token, struct element** first_element, struct syntax_error* error) {
    struct element* element = 0;

    struct token* token = first_token;

    skip_spaces_and_comments(&token);

    while (token) {

        if (element == 0) {
            element = malloc(sizeof(struct element));
            *first_element = element;
        } else {
            element->next = malloc(sizeof(struct element));
            element = element->next;
        }
        element->next = 0;
        element->first_child = 0;

        element_status result;

        switch (token->type) {
        case token_at:
            result = parse_at_rule(token, element, error);
            break;
        case token_dot:
        case token_hash:
        case token_identifier:
        case token_colon:
            result = parse_rule_set(token, element, error);
            break;
        default:
            error->message = "unexpected token";
            error->token = token;
            element_free(*first_element);
            return element_error;
        }

        if (result != element_found) {
            return result;
        }

        token = element->end->next;

        skip_spaces_and_comments(&token);
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
            element_print_tree(child, 0);
        } else {
            print_elements_of_type(element, type);
        }
    }
}

void print_vars(struct element* element) {
    if (element->type == element_declaration) {
        char const* property_source = element->first_child->start->pointer;
        if (strncmp(property_source, "--", 2) == 0) {
            declaration_print(element);
        }
        return;
    }

    for (struct element* child = element->first_child; child; child = child->next) {
        print_vars(child);
    }
}
