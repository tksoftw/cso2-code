#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "split.h"

#define MAX_INPUT_BYTES 5000

char **string_split(const char *input, const char *sep, int *num_words) {
    *num_words = get_word_count(input, sep);

    char **result = malloc((*num_words) * sizeof(char *));
  
    const char *p = input;
    for (int i = 0; i < *num_words; i++) {
        size_t word_len = strcspn(p, sep);
        result[i] = strndup(p, word_len); // OK for POSIX, not Windows

        p += word_len;
        p += strspn(p, sep); // skip sep chars
    }

    return result;
}

int get_word_count(const char *input, const char *sep) {
    int count = 1;

    const char *p = input;
    while (*p != '\0') {
        p += strcspn(p, sep); // goto next sep
        if (*p != '\0') {
            count++;
            p += strspn(p, sep); // skip sep chars
        }
    }

    return count;
}