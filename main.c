#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "0cc.h"

// Report errors
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Report error location
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}


// Token currently focused on
Token *token;

// Input program
char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // トークナイズしてパースする
  // Result weill be stored to code
  user_input = argv[1];
  tokenize();
  program();
  
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // prologue
  // Allocate areas for 26 variables
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");
  // generate codes
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // The last stack should be the evaluation result
    printf("  pop rax\n");
  }

  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
