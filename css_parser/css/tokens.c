#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokens.h"

void free_tokens(struct token* token) {
    if (token->next) {
        free_tokens(token->next);
    }
    free(token);
}

void token_set_string(struct token* token, char const* source, size_t length) {
    char* string = malloc(length + 1);
    if (!string) {
        puts("Failed to allocate memory!");
        exit(1);
    }
    strncpy(string, source, length);
    string[length] = 0;
    token->string = string;
}

struct token* token_get(struct token* first_token, size_t i) {
    struct token* token = first_token;
    for (size_t j = 0; j < i; j++) {
        token = token->next;
    }
    return token;
}

typedef token_status(get_token_function)(struct token*, char const*, size_t, struct lexical_error*);

token_status get_space_token_old(struct token* token, char const* source, size_t pos, struct lexical_error* error) {
    (void)error;

    static regex_t space_regex;
    static int space_regex_comp_result = -1;
    if (space_regex_comp_result == -1) {
        space_regex_comp_result = regcomp(&space_regex, "^[[:space:]]\\{1,\\}", 0);
        if (space_regex_comp_result != 0) {
            fprintf(stderr, "Error compiling space regex. Error code = %d", space_regex_comp_result);
            exit(space_regex_comp_result);
        }
    }

    regmatch_t space_match;
    if (regexec(&space_regex, &source[pos], 1, &space_match, 0) == 0) {
        token->type = token_space;
        token->pointer = &source[pos];
        token->length = space_match.rm_eo - space_match.rm_so;
        token_set_string(token, token->pointer, token->length);
        return token_found;
    } else {
        return token_not_found;
    }
}

token_status get_space_token(struct token* token, char const* source, size_t pos, struct lexical_error* error) {
    (void)error;

    size_t space_length = 0;
    size_t i = pos;
    char current_char;
    while ((current_char = source[i]) != 0) {
        switch (current_char) {
        case ' ':
        case '\n':
        case '\t':
        case '\r':
            space_length++;
            i++;
            continue;
        }
        break;
    }

    if (space_length == 0) {
        return token_not_found;
    }

    token->type = token_space;
    token->pointer = &source[pos];
    token->length = space_length;
    token_set_string(token, token->pointer, token->length);
    return token_found;
}

token_status get_single_char_token(struct token* token, char const* source, size_t pos, struct lexical_error* error) {
    (void)error;

    for (size_t token_type = 0; single_char_tokens[token_type] != 0; token_type++) {
        if (source[pos] == single_char_tokens[token_type]) {
            token->type = token_type;
            token->pointer = &source[pos];
            token->length = 1;
            token_set_string(token, token->pointer, token->length);

            return token_found;
        }
    }

    return token_not_found;
}

token_status get_comment_token(struct token* token, char const* source, size_t pos, struct lexical_error* error) {
    if (strncmp(&source[pos], "/*", 2) != 0) {
        return token_not_found;
    }

    char* comment_end = strstr(&source[pos], "*/");
    if (comment_end == 0) {
        error->message = "unmatched comment start '/*'";
        error->source = source;
        error->pos = pos;
        error->length = 2;
        return token_error;
    }

    token->type = token_comment;
    token->pointer = &source[pos];
    token->length = (comment_end + 2) - (source + pos);
    token_set_string(token, token->pointer, token->length);
    return token_found;
}

token_status get_string_token(struct token* token, char const* source, size_t pos, struct lexical_error* error) {
    char quote = source[pos];
    if (quote != '"' && quote != '\'') {
        return token_not_found;
    }

    char const* end_quote = strchr(&source[pos + 1], quote);
    if (end_quote == 0) {
        error->message = "unmatched quote";
        error->source = source;
        error->pos = pos;
        error->length = 1;

        return token_error;
    }

    token->type = token_string;
    token->pointer = &source[pos];
    token->length = end_quote - &source[pos] + 1;
    token_set_string(token, token->pointer, token->length);
    return token_found;
}

token_status get_number_token_old(struct token* token, char const* source, size_t pos, struct lexical_error* error) {
    (void)error;

    static regex_t number_regex;
    static int number_regex_comp_result = -1;
    if (number_regex_comp_result == -1) {
        number_regex_comp_result = regcomp(&number_regex, "^[[:digit:]]+(\\.[[:digit:]]+)?", REG_EXTENDED);
        if (number_regex_comp_result != 0) {
            fprintf(stderr, "Error compiling space regex. Error code = %d", number_regex_comp_result);
            exit(number_regex_comp_result);
        }
    }

    regmatch_t number_match;
    int result = regexec(&number_regex, &source[pos], 1, &number_match, 0);
    if (result != 0) {
        return token_not_found;
    }

    token->type = token_number;
    token->pointer = &source[pos];
    token->length = number_match.rm_eo - number_match.rm_so;
    token_set_string(token, token->pointer, token->length);
    return token_found;
}

token_status get_number_token(struct token* token, char const* source, size_t pos, struct lexical_error* error) {
    size_t i = pos;
    size_t number_length = 0;
    size_t after_dot_length = 0;
    bool dot = false;
    bool minus = false;
    char current_char;

    if (source[i] == '-') {
        minus = true;
        number_length ++;
        i++;
    }

    while ((current_char = source[i]) != 0) {
        if (isdigit(current_char)) {
            if (dot) {
                after_dot_length++;
            }
            number_length++;
            i++;
            continue;
        }

        if (!dot && current_char == '.') {
            dot = true;
            number_length++;
            i++;
            continue;
        }

        break;
    }

    if (number_length == 0) {
        return token_not_found;
    }

    if (number_length == 1 && dot) {
        return token_not_found;
    }

    if (number_length == 2 && dot && minus) {
        return token_not_found;
    }

    if (dot && after_dot_length == 0) {
        error->message = "expected a number after \".\"";
        error->pos = i + number_length - 1;
        error->source = source;
        error->length = 1;
        return token_error;
    }

    token->type = token_number;
    token->pointer = &source[pos];
    token->length = number_length;
    token_set_string(token, token->pointer, token->length);
    return token_found;
}

token_status get_identifier_token(struct token* token, char const* source, size_t pos, struct lexical_error* error) {
    (void)error;

    size_t i;
    for (i = pos; source[i] != 0; i++) {
        if (i == pos && isdigit(source[i])) {
            return token_not_found;
        }

        if (i == pos + 1 && source[i - 1] == '-' && isdigit(source[i])) {
            return token_not_found;
        }

        if (!isalpha(source[i]) && !isdigit(source[i]) && source[i] != '-' && source[i] != '_') {
            break;
        }
    }

    size_t token_length = i - pos;
    if (token_length < 1 || (token_length == 1 && source[pos] == '-')) {
        return token_not_found;
    }

    token->type = token_identifier;
    token->pointer = &source[pos];
    token->length = token_length;
    token_set_string(token, token->pointer, token->length);
    return token_found;
}

token_status get_token(struct token* token, char const* source, size_t pos, struct lexical_error* error) {
    get_token_function* get_token_functions[] = {
        get_space_token,
        get_comment_token,
        get_identifier_token,
        get_number_token,
        get_string_token,
        get_single_char_token,
        0};

    for (size_t i = 0;; i++) {
        if (get_token_functions[i] == 0) {
            break;
        }

        token_status result = get_token_functions[i](token, source, pos, error);
        switch (result) {
        case token_found:
        case token_error:
            return result;

        case token_not_found:
            continue;
        }
    }

    return token_not_found;
}

token_status parse_tokens(struct token* tokens, size_t* tokens_number, char const* source, struct lexical_error* error) {
    static size_t log_treshold = 1000;

    size_t source_len = strlen(source);

    size_t source_i = 0;
    size_t token_i = 0;
    size_t log_i = log_treshold;
    struct token* token = &tokens[token_i];
    while (source_i < source_len) {
        token->next = &tokens[token_i];
        token = token->next;
        switch (get_token(token, source, source_i, error)) {
        case token_found:
            source_i += token->length;
            token_i++;
            if (token_i > log_i) {
                printf("%zu%%, %zu tokens, %zu chars\n", source_i / (source_len / 100), token_i + 1, source_i + 1);
                log_i += log_treshold;
            }
            continue;
        case token_not_found:
            error->message = "unexpected character";
            error->source = source;
            error->pos = source_i;
            error->length = 1;

            return token_error;
        case token_error:
            return token_error;
        }
    }
    token->next = 0;
    *tokens_number = token_i;

    return token_found;
}
