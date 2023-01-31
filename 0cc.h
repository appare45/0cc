#include <stdbool.h>

// Types of tokens
typedef enum {
  TK_RESERVED,  // Operator
  TK_IDENT,     // Identifier
  TK_NUM,       // Integer
  TK_EOF,       // End of line
} TokenKind;

typedef struct Token Token;

// Token
struct Token {
  TokenKind kind; // Token type
  Token *next;    // Next token
  int val;        // The value of integer
  char *str;      // Letters
  int len;        // Length of token
};

// Token currently focused on
extern Token *token;

// Input program
extern char *user_input;

// Function to report errors
void error(char *fmt, ...);

// Function to report the error location
void error_at(char *loc, char *fmt, ...);


void tokenize();

// Abstract syntax tree node kind
typedef enum
{
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_ASSIGN,  // =
  ND_LVAR,  //Local variable
  ND_NUM, // Integer
  ND_NE,  // !=
  ND_EQ,  // ==
  ND_LT,  // <
  ND_OL,  // <=
} NodeKind;

typedef struct Node Node;

// Node
struct Node {
  NodeKind kind;  // Node Kind
  Node *rhs;      // Right side
  Node *lhs;      // Left side
  int val;        // Use only if the node kind is integer
  int offset;     // Use only if the node kind is local variable
};

extern Node *code[100];
void program();

// Output assembly by using stack machine
void gen(Node *node);