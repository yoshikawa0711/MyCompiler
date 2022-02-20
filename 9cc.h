#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 抽象構文木のノードの種類
typedef enum {
	ND_ADD, 	// +
	ND_SUB, 	// -
	ND_MUL, 	// *
	ND_DIV, 	// /
	ND_EQ,		// ==
	ND_INEQ,	// !=
	ND_LT,		// <
	ND_LTE,		// <=
	ND_ASSIGN, 	// =
	ND_LVAR,	// ローカル変数
	ND_NUM,		// 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
	NodeKind kind; 	// ノードの型
	Node *lhs;		// 左辺
	Node *rhs;		// 右辺
	int val;		// kindがND_NUMの場合のみ使う
	int offset;		// kindがND_LVARの場合のみ使う
};

// トークンの種類
typedef enum {
	TK_RESERVED,	// 記号
	TK_IDENT,		// 識別子
	TK_NUM,			// 整数トークン
	TK_EOF,			// 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
	TokenKind 	kind;	// トークンの型
	Token 		*next;	// 次の入力トークン
	int 		val;	// kindがTK_NUMの場合，その数値
	char 		*str;	// トークン文字列
	int 		len;	// トークンの長さ
};

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

Node *code[100];

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
void error_at(char *loc, char *fmt, ...);

Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

bool starts_with(char *p, char *q);

void gen(Node *node);
void gen_lval(Node *node);

void error(char *fmt, ...);
