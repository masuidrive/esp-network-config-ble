
#include <stdbool.h>
#include <string.h>

#include "tokenizer.h"

#define is_skip_char(chr) (chr == '\r')
#define is_escape_char(chr) (chr == '\\')
#define is_quote_char(chr) (chr == '"')
#define is_end_of_string(chr) (chr == '\0')
#define is_separator(chr) (chr == ' ' || chr == '\n')
#define is_separator_or_null(chr) (is_separator(chr) || is_end_of_string(chr))

char *get_token(char *text, char **token) {
  // skip head spaces
  while (is_separator(*text))
    ++text;

  // empty string
  if (is_end_of_string(*text)) {
    *token = NULL;
    return NULL;
  }

  char *cur;
  if (is_quote_char(*text)) { // quoted
    cur = ++text;             // skip quote char
    *token = text;
    bool escape_char = false;
    while (!((is_quote_char(*text) && !escape_char) || is_end_of_string(*text))) {
      if (is_quote_char(*text) && escape_char) {
        *(cur - 1) = *text++;
        escape_char = false;
      } else {
        escape_char = is_escape_char(*text);
        if (!is_skip_char(*text)) {
          *cur++ = *text;
        }
        ++text;
      }
    }
  } else {
    cur = text;
    *token = text;
    while (!is_separator_or_null(*text)) {
      if (!is_skip_char(*text)) {
        *cur++ = *text;
      }
      ++text;
    }
  }
  int end_of_text = is_end_of_string(*text);
  *cur++ = '\0';

  return end_of_text ? NULL : text + 1;
}
