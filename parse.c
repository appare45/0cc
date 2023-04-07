#include "0cc.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct LVar LVar;

// type of local variable
struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

// loval variables
LVar *locals;

// find local variable by name
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == token->len && !memcmp(tok->str, var->name, var->len))
      return var;
    return NULL;
}


/**
 * \fn void expect(char *op)
 * \brief If the next token is expected symbole, read one token.
 *        Otherwise return an error.
 * \param op the character to be expected
*/
void expect(char *op) {
  if (token->kind != TK_RESERVED || 
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "expected '%c'", op);
  token = token->next;
}

/**
 * \fn bool consume(char *op)
 * \brief If the next token is the expected symbol, read one token and return true.
 *        Otherwise, return false
 * \param op the character to be expected
 * \return  whether the next token is expected
*/
bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len || 
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

/**
 * \fn int expect_number()
 * \brief if the next toke is an integer, read one token and return the value.
 *        Otherwise, report an error.
 * \return the integer, value of the current token
*/
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

/**
 * \fn Token *consume_ident()
 * \brief consume one token and determine if it is an identifier
 * \return if the next toke is an identifier, return the next token and read through the token. otherwise return NULL.
*/
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


/**
 * \fn bool at_eof()
 * \brief determine if the token is eof
 * \return wheter the token is eof or not
*/
bool at_eof() {
  return token->kind == TK_EOF;
}

/**
 * \fn Node *new_node(NodeKind kind)
 * \brief create new node
 * \param kinde the kind of the node to be created
 * \return the created node
*/
Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

/**
 * \fn Node *new_binary(NodeKinde kind, Node *lhs, Node *rhs)
 * \brief create a new binary node
 * \param kind the kind of node to be created
 * \param lhs the node of left hand of the binary
 * \param rhs the node of right hand of the binary
 * \return the created node
*/
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

/**
 * \fn Node *new_num(int val)
 * \brief create a new integer node
 * \param val the value of the node
 * \return the created integer node
 * 
*/
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