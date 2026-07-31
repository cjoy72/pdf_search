#ifndef OCR_H
#define OCR_H

/**
 * Checks whether the Tesseract OCR engine binary is available in system PATH.
 * Caches the result after the first check to avoid redundant system calls.
 *
 * @return 1 if tesseract is available, 0 otherwise.
 */
int check_tesseract_available(void);

/**
 * Renders a single PDF page into a PNG image (`pdftoppm`) and performs OCR (`tesseract`)
 * to search for a case-insensitive keyword on image-based or scanned pages.
 *
 * @param pdf_path Absolute or relative path to the PDF document.
 * @param page The 1-based page index to extract and analyze.
 * @param needle The search term to match against OCR output.
 * @return 1 if the search term was matched via OCR, 0 otherwise.
 */
int ocr_pdf_page(const char *pdf_path, int page, const char *needle);

#endif /* OCR_H */
