#include <stdio.h>
#include <unistd.h>

#include "tokens.h"

void print_help() {
    puts("Usage: css-parser [-f FILE_PATH] [-p PROJECT_PATH] [OPTIONS]");
}

int main(int argc, char* argv[]) {
    char const* file_path;
    char const* project_path;

    int opt;
    while ((opt = getopt(argc, argv, 0)) != -1) {
        switch (opt) {
        case 'f':
            if (!optarg) {
                puts("Please, specify the file path");
                print_help();
                exit(1);
            }

            file_path = optarg;
            break;
        default:
            printf("Invalid option \"%c\"\n", opt);
            print_help();
            exit(1);
        }
    }

    if (!file_path && !project_path) {
        print_help();
        return 1;
    }

    return 0;
}
