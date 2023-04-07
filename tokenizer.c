#include "0cc.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// Create a new token and assign it to cur.next and return the new token
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// Return whether the arguments are same or not
bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

void tokenize() {
  char *p = user_input;
  // Initial token
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // Skip whitespace characters
    if(isspace(*p)) {
      p++;
      continue;
    }

    // Multi-letter operator
    if (startswith(p, "==") ||
        startswith(p, "!=") ||
        startswith(p, "<=") ||
        startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
    }

    // Single-letter operator
    if (strchr("+-*/()<>=;", *p))
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // 数値
    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    // local variable
    if('a' <= *p && *p <= 'z') {
      int len = 0;
      char *str = p;
      while ('a' <= *p && *p <= 'z')
      {
        len++;
        p++;
      }
      cur = new_token(TK_IDENT, cur, str, len);
      continue;
    }
    error_at(p, "トークナイズできません");
  }

  cur = new_token(TK_EOF, cur, p, 0);
  token = head.next;
  return;
}
