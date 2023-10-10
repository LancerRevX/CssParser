#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <css/elements.h>
#include <css/tokens.h>

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

void print_error(char const* source, size_t pos, size_t length, char const* message) {
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

    printf("%zu:%zu\n%s\n", line_number, pos - line_start_i, message);
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

int compare_vars(void const* _left, void const* _right) {
    struct element* left = *((struct element**) _left);
    struct element* right = *((struct element**) _right);
    return strcmp(left->first_child->start->string, right->first_child->start->string);
}

struct options {
    char const* file_path;
    char const* project_path;

    bool print_elements;
    bool print_tokens;
    int filter_tokens;
    bool print_vars;
};

static struct option longopts[] = {
    {"file-path", required_argument, 0, 'f'},
    {"project-path", required_argument, 0, 'p'},
    {"print-elements", no_argument, 0, 'e'},
    {"print-tokens", optional_argument, 0, 't'},
    {"print-vars", no_argument, 0, 'v'},
};

void print_help() {
    puts("Usage: css-parser [OPTIONS]");
    puts("Available options:");
    for (size_t i = 0; i < sizeof(longopts) / sizeof(struct option); i++) {
        if (longopts[i].val) {
            printf("-%c, ", longopts[i].val);
        }
        printf("--%s", longopts[i].name);
        if (longopts[i].has_arg == required_argument || longopts[i].has_arg == optional_argument) {
            printf(" [%s]", longopts[i].name);
        }
        puts("");
    }
}

int parse_command_line(int argc, char* argv[], struct options* options) {
    int opt;
    int option_index;
    while ((opt = getopt_long(argc, argv, "f: p: t:: e", longopts, &option_index)) != -1) {
        switch (opt) {
        case 0:
            break;
        case 'f':
            options->file_path = optarg;
            break;
        case 'p':
            options->project_path = optarg;
            break;
        case 't':
            options->print_tokens = true;
            if (optarg) {
                options->filter_tokens = atoi(optarg);
            }
            break;
        case 'e':
            options->print_elements = true;
            break;
        case 'v':
        options->print_vars = true;
        break;
        default:
            print_help();
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    struct options options = {0, 0, 0, 0, -1, 0};
#ifdef DEBUG
    options.file_path = "d:/projects/atribut-local/wp-content/themes/woodmart/style.css";
    options.print_vars = true;
#else
    parse_command_line(argc, argv, &options);
#endif

    FILE* file;
    struct token* tokens;
    struct element* first_element = 0;
    size_t elements_number = 0;
    size_t tokens_number;
    char* source;
    if (options.file_path) {
        file = fopen(options.file_path, "rb");
        if (!file) {
            printf("Couldn't open file \"%s\"", options.file_path);
            return 1;
        }
        if (fseek(file, 0L, SEEK_END) != 0) {
            puts("fseek error");
            exit(1);
        }
        size_t file_size = ftell(file);
        rewind(file);

        source = calloc(file_size + 1, sizeof(char));
        fread(source, sizeof(char), file_size, file);
        source[file_size] = 0;

        printf("Parsing file \"%s\"...\n", options.file_path);

        tokens = calloc(file_size, sizeof(struct token));
        if (!tokens) {
            puts("Couldn't allocate memory");
        }

        printf("Allocated %zu KiB\n", file_size * sizeof(struct token) / 1024);

        struct lexical_error error;
        enum token_status parse_result =
            parse_tokens(tokens, &tokens_number, source, &error);

        if (parse_result & (token_error | token_not_found)) {
            printf("Lexical error while parsing file \"%s\":", options.file_path);
            print_error(error.source, error.pos, error.length, error.message);
            return 1;
        } else {
            puts("Lexical analysis successful...");
        }

        struct syntax_error syntax_error;
        enum element_status syntax_result = parse_elements(tokens, &first_element, &syntax_error);

        if (syntax_result != element_found) {
            printf("Problem with token \"%s\":\n", token_names[syntax_error.token->type]);
            printf("Syntax error while parsing file \"%s\":", options.file_path);
            print_error(source, syntax_error.token->pointer - source, syntax_error.token->length, syntax_error.message);

            return 1;
        } else {
            puts("Syntax analysis successful...");
        }
    } else {
        print_help();
        return 1;
    }

    if (options.print_tokens) {
        printf("Found %zu tokens\n", tokens_number);
        for (size_t i = 0; i < tokens_number; i++) {
            struct token* token = &tokens[i];

            if (options.filter_tokens && token->type != (enum token_type)options.filter_tokens) {
                continue;
            }

            struct file_pos file_pos;
            calculate_file_pos(source, token->pointer - source, &file_pos);
            printf("%s:%zu:%zu\t\"%s\"\t\t\"%.*s\"\n", options.file_path, file_pos.line, file_pos.line_pos, token_names[token->type], (int)token->length, token->pointer);
        }
    }

    if (options.print_elements) {
        printf("Found elements:\n");
        for (struct element* element = first_element; element; element = element->next) {
            struct file_pos file_pos;
            calculate_file_pos(source, element->start->pointer - source, &file_pos);
            printf("%s:%zu:%zu\t\"%s\"\t\t\"%s\"\n", options.file_path, file_pos.line, file_pos.line_pos, element_name[element->type], element->start->string);
        }
    }

    if (options.print_vars) {
        struct element** vars = calloc(10000, sizeof(struct element));
        size_t var_i = 0;
        get_vars(first_element, vars, &var_i);
        qsort(vars, var_i, sizeof(struct element*), compare_vars);
        printf("Found %zu vars:\n", var_i);
        for (size_t i = 0; i < var_i; i++) {
            struct file_pos file_pos;
            calculate_file_pos(source, vars[i]->start->pointer - source, &file_pos);
            printf("%zu: %s:%zu:%zu === ", i, options.file_path, file_pos.line, file_pos.line_pos);
            declaration_print(vars[i]);
        }        
    }

    return 0;
}
