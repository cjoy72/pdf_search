#ifndef UTILS_H
#define UTILS_H

/**
 * Checks if a string ends with a given suffix.
 *
 * @param str The string to check.
 * @param suffix The suffix to look for at the end of str.
 * @return 1 if str ends with suffix, 0 otherwise.
 */
int ends_with(const char *str, const char *suffix);

/**
 * Converts a string to lowercase characters in place.
 *
 * @param s NUL-terminated string to be modified in place.
 */
void to_lowercase(char *s);

/**
 * Performs a case-insensitive substring search (needle in text).
 *
 * @param text The full string body to search within.
 * @param needle The substring pattern to look for.
 * @return 1 if needle is found inside text (case-insensitive), 0 otherwise.
 */
int contains_case_insensitive(const char *text, const char *needle);

/**
 * Determines whether a filename ends with a .pdf or .PDF extension.
 *
 * @param name The filename or path to inspect.
 * @return 1 if the file extension matches PDF format, 0 otherwise.
 */
int is_pdf_file(const char *name);

#endif /* UTILS_H */
