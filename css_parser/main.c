#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <css/tokens.h>
#include <css/elements.h>

void print_help() {
    puts("Usage: css-parser [-f FILE_PATH] [-p PROJECT_PATH] [OPTIONS]");
}

void print_lexical_error(struct lexical_error const* error) {
    size_t source_length = strlen(error->source);
    size_t line_number = 1;
    size_t line_start_i = 0;
    for (size_t i = 0; i < source_length; i++) {
        if (error->source[i] == '\n') {
            line_number++;
            line_start_i = i + 1;
        }
        if (i == error->pos) {
            break;
        }
    }

    size_t error_log_start_pos = (error->pos - line_start_i) < 16 ? line_start_i : error->pos - 16;
    size_t error_log_end_pos = strstr(&error->source[error_log_start_pos], "\n") - error->source;

    printf("%zu:%zu\n%s\n", line_number, error->pos - line_start_i, error->message);
    for (size_t i = error_log_start_pos; i < error_log_end_pos; i++) {
        switch (error->source[i]) {
            case '\n':
                printf("\\n");
                break;
            case '\t':
                printf("\\t");
                break;
            default:
                printf("%c", error->source[i]);
        }
    }
    puts("");

    for (size_t i = error_log_start_pos; i < error->pos; i++) {
        printf(" ");
        if (error->source[i] == '\n' || error->source[i] == '\t') {
            printf(" ");
        }
    }
    for (size_t i = 0; i < error->length; i++) {
        printf("^");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    char const* file_path = 0;
    char const* project_path = 0;

    int opt;
    while ((opt = getopt(argc, argv, "f:p:")) != -1) {
        switch (opt) {
        case 'f':
            file_path = optarg;
            break;
        case 'p':
            project_path = optarg;
            break;
        default:
            print_help();
            return 1;
        }
    }

    #ifdef DEBUG
    file_path = "d:/projects/atribut-local/wp-content/themes/woodmart-child/style.css";
    #endif

    if (file_path) {
        FILE* file = fopen(file_path, "r");
        if (!file) {
            printf("Couldn't open file \"%s\"", file_path);
            return 1;
        }
        fseek(file, 0L, SEEK_END);
        size_t file_size = ftell(file);
        rewind(file);

        char* source = malloc(file_size);
        fread(source, 1, file_size, file);

        struct token* first_token = NULL;
        size_t tokens_number;
        struct lexical_error error;
        enum token_status parse_result =
            parse_tokens(&first_token, &tokens_number, source, &error);

        if (parse_result & (token_error | token_not_found)) {
            printf("Lexical error while parsing file \"%s\":", file_path);
            print_lexical_error(&error);
            return 1;
        }

        printf("Found %zu tokens\n", tokens_number);
        for (struct token* token = first_token; token != 0; token = token->next) {
            switch (token->type) {

            case token_identifier:
                printf("Token \"%s\": \"%.*s\"\n", token_names[token->type], (int)token->length, token->pointer);
            default:
                continue;
            }
        }
    } else {
        print_help();
        return 1;
    }

    return 0;
}
