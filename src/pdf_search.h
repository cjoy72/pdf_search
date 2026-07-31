#ifndef PDF_SEARCH_H
#define PDF_SEARCH_H

/**
 * Queries the total page count of a PDF file using the `pdfinfo` command.
 *
 * @param pdf_path Path to the target PDF document.
 * @return Total page count of the PDF (defaults to 1 on failure).
 */
int get_page_count(const char *pdf_path);

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
int search_pdf_file(const char *pdf_path, const char *needle, const char *display_path);

/**
 * Recursively traverses a directory structure, searching all PDF files found.
 *
 * @param dir_path Directory path to start traversing.
 * @param needle Keyword to search for.
 * @return 1 if any matching PDF page was found in the directory tree, 0 otherwise.
 */
int search_directory(const char *dir_path, const char *needle);

#endif /* PDF_SEARCH_H */
