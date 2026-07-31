#include "ocr.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int check_tesseract_available(void) {
    static int tesseract_status = -1;
    if (tesseract_status != -1) {
        return tesseract_status;
    }
    int ret = system("PATH=\"/opt/homebrew/bin:/usr/local/bin:$PATH\" which tesseract >/dev/null 2>&1");
    tesseract_status = (ret == 0) ? 1 : 0;
    return tesseract_status;
}

int ocr_pdf_page(const char *pdf_path, int page, const char *needle) {
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
