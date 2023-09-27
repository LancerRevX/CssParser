#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <css/elements.h>
#include <css/tokens.h>

void print_help() {
    puts("Usage: css-parser [-f FILE_PATH] [-p PROJECT_PATH] [OPTIONS]");
    puts("Available options:");
    puts("-e, --element: print found elements;");
    puts("-t [TYPE], --type [TYPE]: filter elements by selected TYPE. If TYPE is omitted, print the list of available element types instead;");
    puts("--tokens:  print found tokens;");
    puts("--vars:    print found vars.");
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

    int print_elements = 0;
    int filter_by_type = 0;
    int element_type = 0;

    int print_tokens = 0;

    struct option longopts[] = {
        {"file-path", required_argument, 0, 'f'},
        {"project-path", required_argument, 0, 'p'},
        {"elements", no_argument, 0, 'e'},
        {"tokens", no_argument, &print_tokens, 1},
        {"type", optional_argument, 0, 't'},
    };

    int opt;
    int option_index;
    while ((opt = getopt_long(argc, argv, "f: p: t:: e::", longopts, &option_index)) != -1) {
        switch (opt) {
        case 0:
            break;
        case 'f':
            file_path = optarg;
            break;
        case 'p':
            project_path = optarg;
            break;
        case 't':
            filter_by_type = 1;
            if (optarg) {
                element_type = atoi(optarg);
            } else {
                element_type = -1;
            }
            break;
        case 'e':
            print_elements = true;
            break;
        default:
            print_help();
            return 1;
        }
    }

#ifdef DEBUG
    file_path = "d:/projects/atribut-local/wp-content/themes/woodmart-child/style.css";
#endif

    FILE* file;
    struct token* first_token;
    struct element* first_element = 0;
    size_t tokens_number;
    size_t found_elements_number;  
    if (file_path) {
        file = fopen(file_path, "r");
        if (!file) {
            printf("Couldn't open file \"%s\"", file_path);
            return 1;
        }
        fseek(file, 0L, SEEK_END);
        size_t file_size = ftell(file);
        rewind(file);

        char* source = malloc(file_size + 1);
        fread(source, 1, file_size, file);
        source[file_size] = 0;

        printf("Parsing file \"%s\"...\n", file_path);

        struct lexical_error error;
        enum token_status parse_result =
            parse_tokens(&first_token, &tokens_number, source, &error);

        if (parse_result & (token_error | token_not_found)) {
            printf("Lexical error while parsing file \"%s\":", file_path);
            print_lexical_error(&error);
            return 1;
        } else {
            puts("Lexical analysis successful...");
        }

        struct syntax_error syntax_error;
        enum element_status syntax_result = parse_elements(first_token, &first_element, &found_elements_number, &syntax_error);

        if (syntax_result != element_found) {
            printf("Syntax error while parsing file \"%s\":", file_path);
            return 1;
        } else {
            puts("Syntax analysis successful...");
        }
    } else {
        print_help();
        return 1;
    }

    if (print_tokens) {
        printf("Found %zu tokens\n", tokens_number);
        for (struct token* token = first_token; token != 0; token = token->next) {
            switch (token->type) {

            case token_identifier:
                printf("Token \"%s\": \"%.*s\"\n", token_names[token->type], (int)token->length, token->pointer);
            default:
                continue;
            }
        }
    }

    if (filter_by_type && element_type == -1) {
        puts("Available element types:");
        for (size_t i = 0; i < element_types_number; i++) {
            printf("%zu: %s\n", i, element_name[i]);
        }
    } else if (print_elements) {

        if (filter_by_type) {
            printf("Found elements of type \"%s\":", element_name[element_type]);
            print_elements_of_type(first_element, element_type);
        } else {
            printf("Found %i elements:\n", elements_number);
            for (struct element* element = first_element; element != 0; element = element->next) {
                element_print_tree(element, 0);
            }
        }
    }









    return 0;
}
