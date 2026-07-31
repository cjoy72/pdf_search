#include "pdf_search.h"
#include "ocr.h"
#include "utils.h"
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int get_page_count(const char *pdf_path) {
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

int search_pdf_file(const char *pdf_path, const char *needle, const char *display_path) {
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

int search_directory(const char *dir_path, const char *needle) {
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
