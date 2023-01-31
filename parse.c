#include "0cc.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


// If the next token is the expected symbol, read one token.
// Otherwise, report an error.
void expect(char *op) {
  if (token->kind != TK_RESERVED || 
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "expected '%c'", op);
  token = token->next;
}

// If the next token is the expected symbol, read one token and return true.
// Otherwise, return false
bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len || 
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// If the next token is an integer, read one token and return the value. Otherwise, report an error.
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

// If the next token is an identifier, read one token and return the token.
// Otherwise return NULL
Token *consume_ident() {
  if (token->kind == TK_IDENT){
    Token *tok = token;
    token = token->next;
    return tok;
  }
  else{
    return NULL;
  }
}


// Whether the token is eof
bool at_eof() {
  return token->kind == TK_EOF;
}

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

    // Single letter variable
    if('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p++, 1);
      continue;
    }
    error_at(p, "トークナイズできません");
  }

  cur = new_token(TK_EOF, cur, p, 0);
  token = head.next;
  return;
}

// Create a new node
Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

// Create a new binary node
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// Create a new integer node
Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *expr();

// primary = "(" expr ")" | num | ident
Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  // Identifier
  Token *tok = consume_ident();
  if (tok)
  {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) * 8;
    return node;
  }

  // そうでなければ数値のはず
  return new_num(expect_number());
}

// unary = ("+" | "-")? unary
Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0), unary());
  return primary();
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();
  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();
  for (;;) 
  {
    if(consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();
  for (;;)
  {
     if (consume("<"))
      node = new_binary(ND_LT, node, add());
     else if(consume("<="))
      node = new_binary(ND_OL, node, add());
     else if(consume(">"))
      node = new_binary(ND_LT, add(), node);
     else if(consume(">="))
      node = new_binary(ND_OL, add(), node);
     else
      return node;
  }
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();
  for (;;)
  {
    if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else if(consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else
      return node;
  } 
}

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  for (;;) 
  {
    if (consume("="))
      node = new_binary(ND_ASSIGN, node, assign());
    else
      return node;
  }
}

// expr = assign
Node *expr() {
  return assign();
}

// stmt = expr ";"
Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

Node *code[100];
// program = stmt*
void program(Token tok) {
  int i = 0;
  while(!at_eof()){
    code[i++] = stmt();
  }
  code[i] = NULL;
}