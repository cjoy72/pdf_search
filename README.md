# pdf_search

A C program that searches PDF files for a case-insensitive text phrase and reports matching file paths and page numbers.

## Features & OCR Capabilities

- **Direct Text Search**: Uses `pdftotext` for fast text extraction on standard PDFs.
- **Automatic OCR Fallback**: If a page is scanned or an image with no embedded text layer, it automatically renders the page to an image (`pdftoppm`) and performs Optical Character Recognition using `tesseract`.
- **Recursive Directory Search**: Recursively traverses folders to search all `.pdf` / `.PDF` files.

---

## Complete List of Dependencies

### 1. Build & Compiler Dependencies
- **C Compiler**: `gcc` or `clang` with C99 support (`-std=c99`).
- **Build Tool**: `make` (GNU Make).
- **C Standard Library**: Standard C99 / POSIX headers (`stdio.h`, `stdlib.h`, `string.h`, `ctype.h`, `dirent.h`, `sys/stat.h`, `sys/types.h`, `unistd.h`).

### 2. Runtime CLI Dependencies
- **Poppler Utilities** (`poppler` package):
  - `pdftotext`: Extracts plain text streams from PDF pages.
  - `pdfinfo`: Reads PDF metadata and page count.
  - `pdftoppm`: Renders scanned PDF pages into PNG images for OCR.
- **Tesseract OCR Engine** (`tesseract` package):
  - `tesseract`: Performs Optical Character Recognition on rendered page images.
- **Tesseract Language Data** (`tesseract-lang` package):
  - Provides trained OCR models and language datasets for multi-language text recognition.

### 3. System Requirements
- **OS**: macOS or Linux.
- **Temporary Storage**: Access to `/private/tmp` (or `/tmp`) for transient page image files during OCR processing.

---

## Installation of Dependencies

### On macOS (using Homebrew)
```sh
brew install poppler tesseract tesseract-lang
```

### On Linux (Ubuntu / Debian)
```sh
sudo apt-get update
sudo apt-get install -y build-essential poppler-utils tesseract-ocr tesseract-ocr-all
```

---

## Build & System Installation

To compile and install `pdf_search` as a system program (compatible with both macOS and Linux):

```sh
make
```

When you run `make`:
1. It automatically checks for and installs missing dependencies (`poppler`, `tesseract`, `tesseract-lang`) via Homebrew or Linux package managers (`apt`, `dnf`, `pacman`).
2. Compiles the `pdf_search` executable.
3. Installs `pdf_search` system-wide to `/usr/local/bin` (asking for `sudo` permissions if needed) so it can be executed from **any directory**.

To uninstall the system binary:
```sh
make uninstall
```

To clean build artifacts:
```sh
make clean
```

---

## Usage

Since `pdf_search` is installed system-wide, you can run it from any directory:

```sh
pdf_search <search-term> [folder]
```

### Examples

Search current directory for a keyword:
```sh
pdf_search BAROI
```

Search a specific folder:
```sh
pdf_search "invoice number" /path/to/folder
```

### Example Output

Matches found via direct text extraction:
```
./documents/report.pdf -> page 3
```

Matches found via OCR fallback:
```
./scanned_doc.pdf -> page 1 (OCR)
```
