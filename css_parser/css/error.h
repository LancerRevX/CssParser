#include <stdlib.h>

#include "tokens.h"

enum error_type {
    lexical_error,
    syntax_error,
};

struct lexical_error {
    const char* message;
    const char* source;
    size_t pos;
    size_t length;
};

void lexical_error_init(struct lexical_error* error) {

}


struct syntax_error {
    char const* message;
    struct token* token;
    size_t pos;
    size_t length;
};

void syntax_error_from_token(struct syntax_error* error, struct token* token, char const* message) {
    
}

struct file_pos {
    size_t line;
    size_t line_pos;
};

void calculate_file_pos(char const* source, size_t pos, struct file_pos* file_pos) {
    size_t source_length = strlen(source);
    size_t line_number = 1;
    size_t line_start_i = 0;
    for (size_t i = 0; i < source_length; i++) {
        if (source[i] == '\n') {
            line_number++;
            line_start_i = i + 1;
        }
        if (i == pos) {
            break;
        }
    }

    file_pos->line = line_number;
    file_pos->line_pos = pos - line_start_i;
}

void print_error(struct error* error, char const* source) {
    size_t pos = -1;
    size_t length;

    switch (error->type) {
    case lexical_error:
        pos = error->pos;
        length = error->length;
        break;
    case syntax_error:
        pos = error->token->pointer - source;
        length = error->token->length;
        break;
    }

    size_t source_length = strlen(source);
    size_t line_number = 1;
    size_t line_start_i = 0;
    for (size_t i = 0; i < source_length; i++) {
        if (source[i] == '\n') {
            line_number++;
            line_start_i = i + 1;
        }
        if (i == pos) {
            break;
        }
    }

    size_t error_log_start_pos = (pos - line_start_i) < 16 ? line_start_i : pos - 16;
    size_t error_log_end_pos = strstr(&source[error_log_start_pos], "\n") - source;
    if ((error_log_end_pos - error_log_start_pos) > 80) {
        error_log_end_pos = error_log_start_pos + 80;
    }

    printf("%zu:%zu\n%s\n", line_number, pos - line_start_i, error->message);
    for (size_t i = error_log_start_pos; i < error_log_end_pos; i++) {
        switch (source[i]) {
        case '\n':
            printf("\\n");
            break;
        case '\t':
            printf("\\t");
            break;
        default:
            printf("%c", source[i]);
        }
    }
    puts("");

    for (size_t i = error_log_start_pos; i < pos; i++) {
        printf(" ");
        if (source[i] == '\n' || source[i] == '\t') {
            printf(" ");
        }
    }
    for (size_t i = 0; i < length; i++) {
        printf("^");
    }
    printf("\n");
}
