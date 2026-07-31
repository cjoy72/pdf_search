/**
 * ============================================================================
 * Project:       pdf_search
 * Description:   Fast PDF text & OCR search tool for macOS & Linux
 * Author:        cjoy72 (cjoy720@gmail.com)
 * Repository:    https://github.com/cjoy72/pdf_search
 * License:       MIT
 * ============================================================================
 */

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * Checks if a string ends with a given suffix.
 *
 * @param str The string to check.
 * @param suffix The suffix to look for at the end of str.
 * @return 1 if str ends with suffix, 0 otherwise.
 */
static int ends_with(const char *str, const char *suffix) {
    size_t len = strlen(str);
    size_t suf_len = strlen(suffix);
    if (suf_len > len) return 0;
    return strcmp(str + len - suf_len, suffix) == 0;
}

/**
 * Converts a string to lowercase characters in place.
 *
 * @param s NUL-terminated string to be modified in place.
 */
static void to_lowercase(char *s) {
    for (size_t i = 0; s[i] != '\0'; ++i) {
        s[i] = (char)tolower((unsigned char)s[i]);
    }
}

/**
 * Performs a case-insensitive substring search (needle in text).
 *
 * @param text The full string body to search within.
 * @param needle The substring pattern to look for.
 * @return 1 if needle is found inside text (case-insensitive), 0 otherwise.
 */
static int contains_case_insensitive(const char *text, const char *needle) {
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

/**
 * Prints CLI usage instructions to standard error.
 *
 * @param prog The executable binary name (argv[0]).
 */
static void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s <search-term> [folder]\n", prog);
}

/**
 * Determines whether a filename ends with a .pdf or .PDF extension.
 *
 * @param name The filename or path to inspect.
 * @return 1 if the file extension matches PDF format, 0 otherwise.
 */
static int is_pdf_file(const char *name) {
    return ends_with(name, ".pdf") || ends_with(name, ".PDF");
}

/**
 * Checks whether the Tesseract OCR engine binary is available in system PATH.
 * Caches the result after the first check to avoid redundant system calls.
 *
 * @return 1 if tesseract is available, 0 otherwise.
 */
static int check_tesseract_available(void) {
    static int tesseract_status = -1;
    if (tesseract_status != -1) {
        return tesseract_status;
    }
    int ret = system("PATH=\"/opt/homebrew/bin:/usr/local/bin:$PATH\" which tesseract >/dev/null 2>&1");
    tesseract_status = (ret == 0) ? 1 : 0;
    return tesseract_status;
}

/**
 * Renders a single PDF page into a PNG image (`pdftoppm`) and performs OCR (`tesseract`)
 * to search for a case-insensitive keyword on image-based or scanned pages.
 *
 * @param pdf_path Absolute or relative path to the PDF document.
 * @param page The 1-based page index to extract and analyze.
 * @param needle The search term to match against OCR output.
 * @return 1 if the search term was matched via OCR, 0 otherwise.
 */
static int ocr_pdf_page(const char *pdf_path, int page, const char *needle) {
    if (!check_tesseract_available()) {
        return 0;
    }

    char img_prefix[] = "/private/tmp/pdf_search_img_XXXXXX";
    int fd = mkstemp(img_prefix);
    if (fd < 0) return 0;
    close(fd);
    remove(img_prefix);

    char img_path[4096];
    snprintf(img_path, sizeof(img_path), "%s.png", img_prefix);

    char command[4096];
    snprintf(command, sizeof(command),
             "PATH=\"/opt/homebrew/bin:/usr/local/bin:$PATH\" pdftoppm -png -singlefile -r 150 -f %d -l %d '%s' '%s' >/dev/null 2>&1",
             page, page, pdf_path, img_prefix);

    if (system(command) != 0) {
        remove(img_path);
        return 0;
    }

    snprintf(command, sizeof(command), "PATH=\"/opt/homebrew/bin:/usr/local/bin:$PATH\" tesseract '%s' stdout 2>/dev/null", img_path);
    FILE *fp = popen(command, "r");
    if (!fp) {
        remove(img_path);
        return 0;
    }

    char line[8192];
    int found = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (contains_case_insensitive(line, needle)) {
            found = 1;
            break;
        }
    }

    pclose(fp);
    remove(img_path);
    return found;
}

/**
 * Queries the total page count of a PDF file using the `pdfinfo` command.
 *
 * @param pdf_path Path to the target PDF document.
 * @return Total page count of the PDF (defaults to 1 on failure).
 */
static int get_page_count(const char *pdf_path) {
    char command[2048];
    FILE *fp;
    char line[256];
    int pages = 1;

    snprintf(command, sizeof(command), "PATH=\"/opt/homebrew/bin:/usr/local/bin:$PATH\" pdfinfo '%s' 2>/dev/null", pdf_path);
    fp = popen(command, "r");
    if (!fp) return 1;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "Pages:", 6) == 0) {
            char *colon = strchr(line, ':');
            if (colon) {
                pages = atoi(colon + 1);
                if (pages < 1) pages = 1;
            }
            break;
        }
    }

    pclose(fp);
    return pages;
}

/**
 * Searches a single PDF file page by page for the specified search term.
 * First uses direct text extraction (`pdftotext`); if a page yields little or no text
 * (scanned or image PDF), it falls back to OCR via `tesseract`.
 *
 * @param pdf_path File system path of the target PDF file.
 * @param needle Search keyword to locate.
 * @param display_path Path string formatted for user-facing output results.
 * @return 1 if at least one matching page was found, 0 otherwise.
 */
static int search_pdf_file(const char *pdf_path, const char *needle, const char *display_path) {
    char command[4096];
    char temp_path[] = "/private/tmp/pdf_search_XXXXXX";
    int fd = mkstemp(temp_path);
    if (fd < 0) {
        perror("mkstemp");
        return 0;
    }
    close(fd);

    int page_count = get_page_count(pdf_path);
    int matched_any = 0;

    for (int page = 1; page <= page_count; ++page) {
        FILE *fp;
        char line[8192];

        snprintf(command, sizeof(command), "PATH=\"/opt/homebrew/bin:/usr/local/bin:$PATH\" pdftotext -f %d -l %d '%s' '%s' >/dev/null 2>&1", page, page, pdf_path, temp_path);
        if (system(command) != 0) {
            continue;
        }

        fp = fopen(temp_path, "r");
        if (!fp) {
            continue;
        }

        size_t non_space_count = 0;
        int page_matched = 0;
        while (fgets(line, sizeof(line), fp) != NULL) {
            for (size_t i = 0; line[i] != '\0'; ++i) {
                if (!isspace((unsigned char)line[i])) {
                    non_space_count++;
                }
            }
            if (contains_case_insensitive(line, needle)) {
                page_matched = 1;
            }
        }
        fclose(fp);

        if (page_matched) {
            printf("%s -> page %d\n", display_path, page);
            matched_any = 1;
            continue;
        }

        // If pdftotext yielded empty/insufficient text on this page, fall back to OCR
        if (non_space_count < 10) {
            if (ocr_pdf_page(pdf_path, page, needle)) {
                printf("%s -> page %d (OCR)\n", display_path, page);
                matched_any = 1;
            } else if (!check_tesseract_available() && non_space_count == 0) {
                static int warned = 0;
                if (!warned) {
                    fprintf(stderr, "[NOTE] Image/scanned page detected. Install 'tesseract' for OCR support (e.g. brew install tesseract).\n");
                    warned = 1;
                }
            }
        }
    }

    remove(temp_path);
    return matched_any;
}

/**
 * Recursively traverses a directory structure, searching all PDF files found.
 *
 * @param dir_path Directory path to start traversing.
 * @param needle Keyword to search for.
 * @return 1 if any matching PDF page was found, 0 otherwise.
 */
static int search_directory(const char *dir_path, const char *needle) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("opendir");
        return 0;
    }

    struct dirent *entry;
    int found_any = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(path, &st) != 0) {
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            found_any |= search_directory(path, needle);
        } else if (S_ISREG(st.st_mode) && is_pdf_file(entry->d_name)) {
            if (search_pdf_file(path, needle, path)) {
                found_any = 1;
            }
        }
    }

    closedir(dir);
    return found_any;
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
