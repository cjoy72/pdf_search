#include "utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int ends_with(const char *str, const char *suffix) {
    size_t len = strlen(str);
    size_t suf_len = strlen(suffix);
    if (suf_len > len) return 0;
    return strcmp(str + len - suf_len, suffix) == 0;
}

void to_lowercase(char *s) {
    for (size_t i = 0; s[i] != '\0'; ++i) {
        s[i] = (char)tolower((unsigned char)s[i]);
    }
}

int contains_case_insensitive(const char *text, const char *needle) {
    size_t nlen = strlen(needle);
    if (nlen == 0) return 1;

    char *buffer = malloc(strlen(text) + 1);
    if (!buffer) return 0;
    strcpy(buffer, text);
    to_lowercase(buffer);

    char *needle_copy = malloc(nlen + 1);
    if (!needle_copy) {
        free(buffer);
        return 0;
    }
    strcpy(needle_copy, needle);
    to_lowercase(needle_copy);

    int found = (strstr(buffer, needle_copy) != NULL);
    free(buffer);
    free(needle_copy);
    return found;
}

int is_pdf_file(const char *name) {
    return ends_with(name, ".pdf") || ends_with(name, ".PDF");
}
