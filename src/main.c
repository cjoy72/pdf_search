/**
 * ============================================================================
 * Project:       pdf_search
 * Description:   Fast PDF text & OCR search tool for macOS & Linux
 * Author:        cjoy72 (cjoy720@gmail.com)
 * Repository:    https://github.com/cjoy72/pdf_search
 * License:       MIT
 * ============================================================================
 */

#include "pdf_search.h"
#include <stdio.h>

/**
 * Prints CLI usage instructions to standard error.
 *
 * @param prog The executable binary name (argv[0]).
 */
static void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s <search-term> [folder]\n", prog);
}

/**
 * CLI Entry point for pdf_search application.
 * Parses command-line arguments and triggers recursive PDF search.
 *
 * @param argc Argument count.
 * @param argv Argument vector (argv[1] = search term, optional argv[2] = folder path).
 * @return 0 on success/completion, 1 on argument usage error.
 */
int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *needle = argv[1];
    const char *start_dir = (argc == 3) ? argv[2] : ".";

    if (!search_directory(start_dir, needle)) {
        printf("No matches found.\n");
    }

    return 0;
}
