#ifndef __9CC_H__
#define __9CC_H__
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_RESERVED,
  TK_IDENT,
  TK_NUM,
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_RETURN,
  TK_FOR,
  TK_SIZEOF,
  TK_INT,
  TK_STR,
  TK_CHAR,
  TK_STRUCT,
  TK_DOT,
  TK_ARROW,
  TK_EOF
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_MOD,
  ND_NUM,
  ND_EQ,
  ND_NEQ,
  ND_LT,
  ND_LEQ,
  ND_ASSIGN,
  ND_ADDR,
  ND_DEREF,
  ND_LVAR,
  ND_FUNC,
  ND_GVAR,
  ND_STR,
  ND_CALL,
  ND_IF,
  ND_IFELSE,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_RETURN,
  ND_NOP,
} NodeKind;

typedef struct Node Node;
typedef struct Block Block;
typedef struct Variable Variable;
typedef struct Str Str;
typedef struct Arg Arg;
typedef struct Struct Struct;
typedef struct Type Type;
typedef struct Member Member;

typedef enum {
  INT,
  CHAR,
  PTR,
  ARRAY,
  STRUCT,
  VOID,
} TypeKind;

struct Type {
  TypeKind ty;
  Type *base;
  Member *members;
  int array_size; // tyがARRAYだった場合、その要素数 例えばint
                  // a[2][3][4]のとき、a[0]は3,a[0][0]は4
};

struct Member {
  Member *next;
  Type *ty;
  char *name;
  int len;
  int offset;
};

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  Node *target;
  Node *condition;
  Node *true_statement;
  Node *false_statement;
  Node *initialize;
  Node *update;
  Node *statement;

  Block *block;
  int val;         // kindがND_NUMの場合のみ使う
  int offset;      // kindがND_LVARの場合のみ使う
  char *gvarname;  // kindがND_GVALの場合のみ使う
  int gvarnamelen; // kindがND_GVALの場合のみ使う
  char *funcname;  // kindがND_CALLまたはND_FUNCの場合のみ使う
  int funcnamelen; // kindがND_CALLまたはND_FUNCの場合のみ使う
  Arg *args;       // kindがND_CALLの場合のみ使う
  int argnum;      // kindがND_FUNCの場合のみ使う
  Type *type;      // そのノードを処理した結果の型を持つ
  Type *type2; // ノードの型（いずれはこちらに切り替える）
  Str *str;    // kindがND_STRの場合のみ使う
};
struct Block {
  Node *stmt_node;
  Block *next;
};

struct Variable {
  Variable *next;
  char *name;
  int len;
  int offset;
  int size;
  Type *type;
};

struct Struct {
  Struct *next;
  char *name;
  int len;
  Type *type;
};

struct Str {
  Str *next;
  char *str;
  int len;
  int id;
};

struct Arg {
  Node *node;
  Arg *next;
};

void error(char *fmt, ...);

void program();
Type *decl_type();
Node *toplevel();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();
void gen(Node *node);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Token *tokenize(char *p);
Node *expr();

int is_numeric(Type *type);

void gen_globals();
void gen_strings();
void gen_gval(Node *node);

Variable *find_variable(Token *tok, Variable *root);
Member *find_member(Member *head, char *name);
Struct *find_struct(Token *tok);
Block *new_block();
Block *next_block(Block *block);
Arg *new_arg(Arg *cur);
Arg *next_arg(Arg *cur);
void print_type(Type *type);

int type_size(Type *type);
int get_array_offset(Type *type);
char *read_file(char *path);
int get_array_dimesion(Type *type);

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void node_name(NodeKind kind, char *s);
void dumpAST(FILE *fp, Node *node);
extern Token *token;
extern char *user_input;
extern Node *code[1024];
extern Variable *func_variables[256];
extern Variable *locals;
extern Variable *globals;
extern Str *strings;
extern Struct *structs;
extern int label;
extern char **arg_reg_32;
extern char **arg_reg_64;
#endif