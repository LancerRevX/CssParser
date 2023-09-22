#pragma once

#include <ctype.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum token_type {
    token_block_start,
    token_block_end,
    token_bracket_start,
    token_bracket_end,
    token_parentheses_start,
    token_parentheses_end,
    token_semicolon,
    token_colon,
    token_comma,
    token_hash,
    token_dot,
    token_at,
    token_exclamation,
    token_percent,

    token_space,
    token_comment,
    token_identifier,
    token_string,
    token_number,
};

char const single_char_tokens[] = {
    [token_block_start] = '{',
    [token_block_end] = '}',
    [token_bracket_start] = '[',
    [token_bracket_end] = ']',
    [token_parentheses_start] = '(',
    [token_parentheses_end] = ')',
    [token_semicolon] = ';',
    [token_colon] = ':',
    [token_comma] = ',',
    [token_hash] = '#',
    [token_dot] = '.',
    [token_at] = '@',
    [token_exclamation] = '!',
    [token_percent] = '%',
    0};

char const* const token_names[] = {
    [token_block_start] = "Block start",
    [token_block_end] = "Block end",
    [token_bracket_start] = "[",
    [token_bracket_end] = "]",
    [token_parentheses_start] = "(",
    [token_parentheses_end] = ")",
    [token_semicolon] = "Semicolon",
    [token_colon] = "Colon",
    [token_comma] = "Comma",
    [token_hash] = "Hash",
    [token_dot] = "Dot (.)",
    [token_at] = "@",
    [token_exclamation] = "Exclamation (!)",
    [token_percent] = "Percent sign (%)",

    [token_space] = "Space",
    [token_comment] = "Comment",
    [token_identifier] = "Identifier",
    [token_string] = "Quoted string",
    [token_number] = "Number",
    0};

typedef struct token {
    enum token_type type;
    const char* pointer;
    size_t length;

    struct token* next;
} css_token;

typedef struct lexical_error {
    const char* message;
    const char* source;
    size_t pos;
    size_t length;
} css_lexical_error;

typedef enum token_status {
    token_found,
    token_not_found,
    token_error
} token_status;

typedef token_status(get_token_function)(struct token*, char const*, size_t, struct lexical_error*);

void free_tokens(struct token** first_token);

get_token_function get_space_token;
get_token_function get_single_char_token;
get_token_function get_comment_token;
get_token_function get_string_token;
get_token_function get_number_token;
get_token_function get_identifier_token;

token_status get_token(struct token* token, char const* source, size_t pos, struct lexical_error* error);

token_status parse_tokens(struct token** first_token, size_t* tokens_number, char const* source, struct lexical_error* error);
