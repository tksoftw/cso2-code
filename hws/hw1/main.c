#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "split.h"

#define MAX_INPUT_BYTES 5000

void print_words(char **words, int num_words) {
    for (int i = 0; i < num_words; i++) {
        printf("[%s]", words[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    char *sep;

    if (argc == 1) sep = strdup(" \t");
    else { // concatenate all arguments as a separator
        size_t sep_length = 0;
        for (int i = 1; i < argc; i++) sep_length += strlen(argv[i]);
        sep = malloc(sep_length + 1);
        sep[0] = '\0';
        for (int i = 1; i < argc; i++) strcat(sep, argv[i]);
    }
    
    char buffer[MAX_INPUT_BYTES];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, ".") == 0) break; // break on '.'

        int num_words = 0;
        char **words = string_split(buffer, sep, &num_words);
        print_words(words, num_words);

        for (int i = 0; i < num_words; i++) {
            free(words[i]);
        }
        free(words);
    }

    free(sep);
    return 0;
}