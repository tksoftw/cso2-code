#ifndef SPLIT_H
#define SPLIT_H


char **string_split(const char *input, const char *sep, int *num_words);

int get_word_count(const char *input, const char *sep);

#endif