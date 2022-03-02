#include "9cc.h"

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

void program() {
	int i = 0;
	while (!at_eof()) {
		code[i++] = stmt();
	}
	code[i] = NULL;
}

Node *stmt() {
	Node *node;

	if (token->kind == TK_RETURN) {
		token = token->next;
		node = calloc(1, sizeof(Node));
		node->kind = ND_RETURN;
		node->lhs = expr();
	} 
	else {
		node = expr();
	}

	if (!consume(";")) {
		error_at(token->str, "';'ではないトークンです。");
	}

	return node;
}

Node *expr() {
	Node *node = assign();

	return node;
}

Node *assign() {
	Node *node = equality();

	if (consume("=")) { 
		node = new_node(ND_ASSIGN, node, assign());
	}

	return node;
}

Node *equality() {
	Node *node = relational();

	for(;;) {
		if (consume("==")) {
			node = new_node(ND_EQ, node, relational());
		}
		else if (consume("!=")) {
			node = new_node(ND_INEQ, node, relational());
		}
		else {
			return node;
		}
	}
}

Node *relational() {
	Node *node = add();

	for(;;) {
		if (consume("<=")) {
			node = new_node(ND_LTE, node, add());
		}
		else if (consume("<")) {
			node = new_node(ND_LT, node, add());
		}
		else if (consume(">=")) {
			node = new_node(ND_LTE, add(), node);
		}
		else if (consume(">")) {
			node = new_node(ND_LT, add(), node);
		}
		else {
			return node;
		}
	}
}

Node *add() {
	Node *node = mul();

	for(;;) {
		if (consume("+")) {
			node = new_node(ND_ADD, node, mul());
		}
		else if (consume("-")) {
			node = new_node(ND_SUB, node, mul());
		}
		else {
			return node;
		}
	}
}

Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume("*")) {
			node = new_node(ND_MUL, node, unary());
		}
		else if (consume("/")) {
			node = new_node(ND_DIV, node, unary());
		}
		else {
			return node;
		}
	}
}

Node *unary() {
	if (consume("+")) {
		return unary();
	}
	else if (consume("-")) {
		return new_node(ND_SUB, new_node_num(0), unary());
	}

	return primary();
}

Node *primary() {
	// 次のトークンが"("なら，"(" expr ")"のはず
	if (consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}

	// 次のトークンが変数の場合
	// TODO: TK_IDENTかの確認をこことcosume_ident両方でやっているのを修正する
	if (token->kind == TK_IDENT) {	
		Token *tok = consume_ident();
		if (tok) {
			Node *node = calloc(1, sizeof(Node));
			node->kind = ND_LVAR;

			LVar *lvar = find_lvar(tok);
			if (lvar) {
				node->offset = lvar->offset;
			}
			else {	
				lvar = calloc(1, sizeof(LVar));
				if (locals) {
					lvar->next = locals;
					lvar->name = tok->str;
					lvar->len = tok->len;
					lvar->offset = locals->offset + 8;
					node->offset = lvar->offset;
					locals = lvar;
				}
				else { // localsがNULLだった場合
					lvar->name = tok->str;
					lvar->len = tok->len;
					lvar->offset = 8;
					node->offset = lvar->offset;
					locals = lvar;
				}
			}
			return node;
		}
	}

	// そうでなければ数値のはず
	return new_node_num(expect_number());
}

// 次のトークンが期待している記号の時には，トークンを１つ読み進めて
// 真を返す．それ以外の場合には偽を返す．
bool consume(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len)) {
		return false;
	}

	token = token->next;
	return true;
}

// 次のトークンが期待している変数の時には，トークンを１つ読み進めて
// トークンを返す
Token *consume_ident() {
	if (token->kind != TK_IDENT) {
		error_at(token->str, "変数ではありません");
	}

	Token *tok = token;
	token = token->next;
	
	return tok;
}

// 次のトークンが期待している記号の時には，トークンを１つ読み進める．
// それ以外の場合にはエラーを報告する．
void expect(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len)) {
		error_at(token->str, "%sではありません", op);
	}	

	token = token->next;
}

// 次のトークンが数値の場合，トークンを１つ読み進めてその数値を返す．
// それ以外の場合にはエラーを報告する．
int expect_number() {	
	if (token->kind != TK_NUM) {
		error_at(token->str, "数ではありません");
	}

	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 変数名を名前で検索する
// 見つからなかった場合はNULLを返す
LVar *find_lvar(Token *tok) {
	for (LVar *var = locals; var; var = var->next) {
		if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
			return var;
		}
	}

	return NULL;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize() {
	char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		// 空白文字をスキップ
		if (isspace(*p)) {
			p++;
			continue;
		}
		
		if (starts_with(p, ">=") || starts_with(p, "<=") ||
			starts_with(p, "==") || starts_with(p, "!=")) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}	
		
		if (strchr("+-*/()><;=", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
			cur = new_token(TK_RETURN, cur, p, 6);
			p += 6;
			continue;
		}
		
		if (is_alpha(p[0])) {
			char *q;
			for (q = p; is_alnum(p[0]); p++) {
				// 英数字の間だけ読み進める
			}
			cur = new_token(TK_IDENT, cur, q, 0);
			cur->len = p - q;
			continue;
		}

		if (isdigit(*p)) {
			char *q = p;
			cur = new_token(TK_NUM, cur, p, 0);
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}
		
		error_at(p, "トークナイズできません");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

bool starts_with(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

bool is_alnum(char c) {
	if (('a' <= c && c <= 'z') ||
		('A' <= c && c <= 'Z') ||
		('0' <= c && c <= '9') ||
		(c == '_') ) {
		return true;
	}

	return false;
}

bool is_alpha(char c) {
	if (('a' <= c && c <= 'z') ||
		('A' <= c && c <= 'Z') ) {
		return true;
	}

	return false;
}
