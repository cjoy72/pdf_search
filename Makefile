CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -Isrc
TARGET = pdf_search
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.c)

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

UNAME_S := $(shell uname -s)

all: deps $(TARGET) install

deps:
	@echo "Checking dependencies on $(UNAME_S)..."
	@if ! command -v pdftotext >/dev/null 2>&1 || ! command -v tesseract >/dev/null 2>&1; then \
		echo "Missing dependencies detected. Installing required tools..."; \
		if [ "$(UNAME_S)" = "Darwin" ]; then \
			if command -v brew >/dev/null 2>&1; then \
				brew install poppler tesseract tesseract-lang || true; \
			else \
				echo "Error: Homebrew is required on macOS to install dependencies. Please install Homebrew (https://brew.sh)."; \
				exit 1; \
			fi; \
		elif [ "$(UNAME_S)" = "Linux" ]; then \
			if command -v apt-get >/dev/null 2>&1; then \
				sudo apt-get update && sudo apt-get install -y poppler-utils tesseract-ocr tesseract-ocr-all; \
			elif command -v dnf >/dev/null 2>&1; then \
				sudo dnf install -y poppler-utils tesseract tesseract-langpack-en; \
			elif command -v pacman >/dev/null 2>&1; then \
				sudo pacman -Sy --noconfirm poppler tesseract tesseract-data-eng; \
			else \
				echo "Package manager not recognized. Please install poppler and tesseract manually."; \
			fi; \
		else \
			echo "Unsupported OS: $(UNAME_S). Please install poppler and tesseract manually."; \
		fi; \
	else \
		echo "All required dependencies (pdftotext, tesseract) are installed."; \
	fi

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

install: $(TARGET)
	@echo "Installing $(TARGET) system-wide to $(BINDIR)..."
	@if [ -w "$(BINDIR)" ] || { [ ! -e "$(BINDIR)" ] && [ -w "$$(dirname $(BINDIR))" ]; }; then \
		mkdir -p $(BINDIR) && install -m 755 $(TARGET) $(BINDIR)/$(TARGET); \
	else \
		echo "Elevated permissions required. Installing to $(BINDIR) via sudo..."; \
		sudo mkdir -p $(BINDIR) && sudo install -m 755 $(TARGET) $(BINDIR)/$(TARGET); \
	fi
	@echo "Successfully installed $(TARGET) to $(BINDIR)/$(TARGET)."
	@echo "You can now run '$(TARGET) <keyword> [folder]' from any directory."

run: $(TARGET)
	./$(TARGET) $(ARGS)

uninstall:
	@echo "Uninstalling $(TARGET) from $(BINDIR)..."
	@if [ -w "$(BINDIR)/$(TARGET)" ]; then \
		rm -f $(BINDIR)/$(TARGET); \
	else \
		sudo rm -f $(BINDIR)/$(TARGET); \
	fi
	@echo "Successfully uninstalled $(TARGET)."

clean:
	rm -f $(TARGET)

.PHONY: all deps install uninstall run clean
