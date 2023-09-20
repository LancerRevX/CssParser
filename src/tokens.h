#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum token_type {
    token_space,
    token_comment,
    token_block_start,
    token_block_end,
    token_semicolon,
    token_colon,
    token_comma,
    token_string,
};

struct token {
    enum token_type type;
    const char* pointer;
    size_t length;
};

struct lexical_error {
    const char* message;
    const char* source;
    size_t pos;
    size_t length;
};

enum token_status {
    token_found,
    token_not_found,
    token_error
};

enum token_status get_space_token(struct token* token, char const* source, size_t pos);
enum token_status get_single_char_token(struct token* token, char const* source, size_t pos);
enum token_status get_comment_token(struct token*, char const*, size_t, struct lexical_error*);
enum token_status get_string_token(struct token* token, char const* source, size_t pos, struct lexical_error*);

enum token_status parse_tokens(struct token tokens[], size_t* tokens_number, char const* source, struct lexical_error* error) {
    size_t source_len = strlen(source);

    size_t i = 0;
    struct token* token = tokens;
    *tokens_number = 0;
    while (i < source_len) {
        if (get_space_token(token, source, i) == token_found) {
            goto next_token;
        }

        if (get_single_char_token(token, source, i) == token_found) {
            goto next_token;
        }

        enum token_status get_comment_result = get_comment_token(token, source, i, error);
        if (get_comment_result == token_found) {
            goto next_token;
        } else if (get_comment_result == token_error) {
            return token_error;
        }

        enum token_status get_value_result = get_string_token(token, source, i, error);
        if (get_value_result == token_found) {
            goto next_token;
        } else if (get_value_result == token_error) {
            return token_error;
        }

        return token_not_found;

    next_token:
        i += token->length;
        token++;
        (*tokens_number)++;
    }

    return token_found;
}

enum token_status get_space_token(struct token* token, char const* source, size_t pos) {
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
        return token_found;
    } else {
        return token_not_found;
    }
}

enum token_status get_single_char_token(struct token* token, char const* source, size_t pos) {
    enum token_type token_type;

    switch (source[pos]) {
    case '{':
        token_type = token_block_start;
        break;
    case '}':
        token_type = token_block_end;
        break;
    case ';':
        token_type = token_semicolon;
        break;
    case ':':
        token_type = token_colon;
        break;
    case ',':
        token_type = token_comma;
        break;
    default:
        return token_not_found;
    }

    token->type = token_type;
    token->pointer = &source[pos];
    token->length = 1;

    return token_found;
}

enum token_status get_comment_token(struct token* token, char const* source, size_t pos, struct lexical_error* error) {
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
    return token_found;
}

enum token_status get_string_token(struct token* token, char const* source, size_t pos, struct lexical_error* error) {
    static regex_t space_regex;
    static int space_regex_comp_result = -1;
    if (space_regex_comp_result == -1) {
        space_regex_comp_result = regcomp(&space_regex, "^[[:space:]]", 0);
        if (space_regex_comp_result != 0) {
            fprintf(stderr, "Error compiling space regex. Error code = %d", space_regex_comp_result);
            exit(space_regex_comp_result);
        }
    }

    char parentheses[strlen(&source[pos])];
    size_t parentheses_num = 0;
    size_t last_parenthesis_i = -1;
    int last_non_space_char_i = -1;
    size_t i = pos;
    while (i < strlen(source)) {
        char current_char = source[i];
        switch (current_char) {
        case '(':
        case '[':
            parentheses[parentheses_num] = current_char;
            last_parenthesis_i = i;
            parentheses_num++;
            break;
        case ')':
            if (parentheses[parentheses_num - 1] == '(') {
                parentheses_num--;
            } else {
                error->message = "unmatched parenthesis ')'";
                error->pos = i;
                error->length = 1;
                return token_error;
            }
            break;
        case ']':
            if (parentheses[parentheses_num - 1] == '[') {
                parentheses_num--;
            } else {
                error->message = "unmatched parenthesis ']'";
                error->pos = i;
                error->length = 1;
                return token_error;
            }
            break;
        case ';':
        case '{':
        case '}':
        case ',':
        case ':':
            if (parentheses_num == 0) {
                goto return_token;
            }
        }

        if (regexec(&space_regex, &source[i], 0, 0, 0) != 0) {
            last_non_space_char_i = i;
        }

        i++;
    }

return_token:

    if (last_non_space_char_i == -1) {
        return token_not_found;
    }

    if (parentheses_num > 0) {
        error->message = "unmatched starting parenthesis";
        error->pos = last_parenthesis_i;
        error->length = 1;
        return token_error;
    }

    token->type = token_string;
    token->pointer = &source[pos];
    token->length = last_non_space_char_i - pos + 1;
    return token_found;
}
