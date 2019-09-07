#include "9cc.h"
Token *token;
char *user_input;
Node *code[1024];
LVar *func_variables[256];
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{

    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    node->type = calloc(1, sizeof(Type));
    //fprintf(stderr, "%d %d\n", lhs->type->ty, rhs->type->ty);
    //fprintf(stderr, "%s\n", token->str);
    if (node->kind == ND_ADD || node->kind == ND_SUB)
    {
        //  fprintf(stderr, "%d %d\n", lhs->kind, rhs->kind);
        //print_type(node->lhs->type);
        if (lhs->type->ty == PTR && rhs->type->ty == PTR)
        {
            error("ポインタ同士の加算/減算です:%s", token->str);
        }
        if (lhs->type->ty == INT && rhs->type->ty == INT)
        {

            node->type->ty = INT;
        } //char入れたらこの後にも追加する
        else if ((lhs->type->ty == ARRAY && rhs->type->ty == INT) || (lhs->type->ty == INT && rhs->type->ty == ARRAY))
        {
            node->type->ty = ARRAY;
        }
        else
        {
            node->type->ty = PTR;
            if (lhs->type->ty == INT)
            {
                node->type = rhs->type;
            }
            else
            {
                node->type = lhs->type;
            }
        }
    }

    return node;
}

void print_type(Type *type)
{
    switch (type->ty)
    {
    case INT:
        fprintf(stderr, "INT\n");
        return;
    case PTR:
        fprintf(stderr, "PTR->");
        print_type(type->ptr_to);
        return;
    case ARRAY:
        fprintf(stderr, "ARRAY->");
        print_type(type->ptr_to);
        return;
    }
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    node->type = calloc(1, sizeof(Type));
    node->type->ty = INT; // calloc(1, sizeof(Type));
    return node;
}

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    {
        return false;
    }
    token = token->next;
    return true;
}

bool consume_tokenkind(TokenKind kind)
{
    if (token->kind != kind)
    {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_ident()
{
    if (token->kind != TK_IDENT)
    {
        return NULL;
    }
    Token *tok = token;
    token = token->next;
    return tok;
}

void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    {
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
}

int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}
void next_token()
{
    if (at_eof())
    {
        error("プログラムの終端でnext_token()が呼ばれました\n");
    }
    token = token->next;
}

//現在のトークンのstrがopで指定したものか調べる関数
bool check_token(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    {
        return false;
    }
    return true;
}

bool is_alnum(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

//lenはTK_RESERVED(記号)のときに使い、記号の文字長を表す。それ以外のときは-1を入れておく
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}
Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        //fprintf(stderr, "%c\n", *p);
        if (isspace(*p))
        {
            p++;
            continue;
        }
        if ((memcmp(p, "==", 2) == 0) || (memcmp(p, "!=", 2) == 0) || (memcmp(p, "<=", 2) == 0) || (memcmp(p, ">=", 2) == 0))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        else if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '%' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == '=' || *p == ';' || *p == '{' || *p == '}' || *p == ',' || *p == '&' || *p == '[' || *p == ']')
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        else if ((strncmp(p, "if", 2) == 0) && !is_alnum(p[2]))
        {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }
        else if ((strncmp(p, "else", 4) == 0) && !is_alnum(p[4]))
        {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }
        else if ((strncmp(p, "while", 5) == 0) && !is_alnum(p[5]))
        {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }
        else if ((strncmp(p, "for", 3) == 0) && !is_alnum(p[3]))
        {
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }
        else if ((strncmp(p, "sizeof", 6) == 0) && !is_alnum(p[6]))
        {
            cur = new_token(TK_SIZEOF, cur, p, 6);
            p += 6;
            continue;
        }
        else if ((strncmp(p, "return", 6) == 0) && !is_alnum(p[6]))
        {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }
        else if ((strncmp(p, "int", 3) == 0) && !is_alnum(p[3]))
        {
            cur = new_token(TK_INT, cur, p, 3);
            p += 3;
            continue;
        }
        else if (isalpha(*p))
        {
            int len = 0;
            // is_alnumは自作した関数で、標準で用意されているisalnumではない(前者は引数が'_'の場合もtrueを返す)
            while (is_alnum(*(p + len)))
            {
                len++;
            }
            cur = new_token(TK_IDENT, cur, p, len);
            p += len;
            continue;
        }
        else if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, -1);
            cur->val = strtol(p, &p, 10);
            continue;
        }
        error_at(token->str, "トークナイズできません");
    }
    new_token(TK_EOF, cur, p, -1);
    return head.next;
}

void program()
{

    int i = 0;
    globals = NULL;
    while (!at_eof())
    {

        locals = calloc(1, sizeof(LVar));
        locals->offset = 8;
        code[i] = toplevel();
        //トップレベルには関数かグローバル変数の定義が入る。
        //グローバル変数の場合、戻り値はNULLになる（宣言は新しいnodeを生成しないので）
        //返った結果がNULLかどうかで処理を分けている
        if (code[i] != NULL)
        {
            func_variables[i] = locals;
            i += 1;
        }
    }

    code[i] = NULL;
}
Type *decl_type()
{
    //int型でいいのか…？
    int base_type = token->kind;
    next_token();
    Type *type = calloc(1, sizeof(Type));
    Type *cur = type;
    while (consume("*"))
    {
        cur->ty = PTR;
        Type *next = calloc(1, sizeof(Type));
        cur->ptr_to = next;
        cur = next;
    }
    switch (base_type)
    {
    case TK_INT:
        cur->ty = INT;
        break;
    }
    return type;
}

void decl_lvar()
{
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->type = decl_type();
    lvar->next = locals;
    lvar->name = token->str;
    lvar->len = token->len;
    next_token();
    int size = 1;

    if (consume("["))
    {
        Type *next = calloc(1, sizeof(Type));
        next->ty = ARRAY;
        next->ptr_to = lvar->type;
        lvar->type = next;

        size = token->val;
        next_token();
        expect("]");
    }
    if (lvar->type->ty == INT)
    {

        lvar->offset = locals->offset + 4 * size;
    }
    else
    {
        lvar->offset = locals->offset + 8 * size;
    }
    //node->offset = lvar->offset;

    locals = lvar;
    expect(";");
}

Node *toplevel()
{
    Type *type = decl_type();

    Node *node = calloc(1, sizeof(Node));
    char *name = token->str;
    int len = token->len;
    next_token();

    if (consume("("))
    {

        node->kind = ND_FUNC;
        node->funcname = name;
        node->funcnamelen = len;
        node->argnum = 0;
        Node *cur = node;
        if (!consume(")"))
        {
            for (;;)
            {
                LVar *lvar = calloc(1, sizeof(LVar));
                lvar->type = decl_type();
                lvar->next = locals;
                lvar->name = token->str;
                lvar->len = token->len;
                lvar->offset = locals->offset + 4;
                locals = lvar;
                node->argnum += 1;
                next_token();

                if (!consume(","))
                {
                    break;
                }
            }
            expect(")");
        }
        node->rhs = stmt();
        return node;
    }
    else
    {
        LVar *lvar = calloc(1, sizeof(LVar));
        lvar->type = type;
        lvar->next = globals;
        lvar->name = name;
        lvar->len = len;
        int size = 1;
        if (consume("["))
        {
            Type *next = calloc(1, sizeof(Type));
            next->ty = ARRAY;
            next->ptr_to = lvar->type;
            lvar->type = next;

            size = token->val;
            next_token();
            expect("]");
        }
        if (lvar->type->ty == INT)
        {

            lvar->size = 4 * size;
        }
        else
        {
            lvar->size = 8 * size;
        }
        globals = lvar;
        expect(";");
        return NULL;
    }
}

Node *stmt()
{

    Node *node;
    if (consume_tokenkind(TK_RETURN))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
        return node;
    }
    else if (consume_tokenkind(TK_IF))
    {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->lhs = expr();
        expect(")");
        Node *node_stmt_true = stmt();
        if (consume_tokenkind(TK_ELSE))
        {
            node->kind = ND_IFELSE;
            node->rhs = new_node(ND_IFELSE, node_stmt_true, stmt());
        }
        else
        {
            node->rhs = node_stmt_true;
        }
        return node;
    }
    else if (consume_tokenkind(TK_WHILE))
    {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->lhs = expr();
        expect(")");
        node->rhs = stmt();
        return node;
    }
    else if (consume_tokenkind(TK_FOR))
    {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;

        //for (A;B;C) D; のA,Bを格納するノード
        Node *node_init_cond = calloc(1, sizeof(Node));

        //for (A;B;C) D; のC,Dを格納するノード
        Node *node_update_stmt = calloc(1, sizeof(Node));
        if (!check_token(";"))
        {
            node_init_cond->lhs = expr();
        }
        else
        {

            node_init_cond->lhs = NULL;
        }
        consume(";");

        if (!check_token(";"))
        {
            node_init_cond->rhs = expr();
        }
        else
        {
            node_init_cond->rhs = NULL;
        }
        consume(";");

        if (!check_token(")"))
        {
            node_update_stmt->lhs = expr();
        }
        else
        {
            node_update_stmt->lhs = NULL;
        }
        consume(";");

        expect(")");
        node_update_stmt->rhs = stmt();
        node->lhs = node_init_cond;
        node->rhs = node_update_stmt;
        return node;
    }
    else if (consume("{"))
    {

        /*
        BLOCK=
            "{" (declvar* stmt declvar*)* "}" |
            "{" (declvar*) "}"
        という構文になる
        */
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->block = calloc(1, sizeof(Block));

        Block *current_block = node->block;
        while (token->kind == TK_INT)
        {
            decl_lvar();
        }
        if (!(consume("}")))
        {
            for (;;)
            {
                while (token->kind == TK_INT)
                {
                    decl_lvar();
                }
                current_block->stmt_node = stmt();

                while (token->kind == TK_INT)
                {
                    decl_lvar();
                }

                if (consume("}"))
                    break;
                current_block = new_block(current_block);
            }
            current_block->next = NULL;
        }
        else
        {
            node->block = NULL;
        }
        return node;
    }
    else
    {
        node = expr();
        expect(";");
        return node;
    }
    error("stmtで解釈できません:%s\n", token->str);
}

Node *expr()
{
    return assign();
}

Node *assign()
{
    Node *node = equality();
    if (consume("="))
    {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume("=="))
        {

            node = new_node(ND_EQ, node, relational());
        }
        else if (consume("!="))
        {
            node = new_node(ND_NEQ, node, relational());
        }
        else
        {
            return node;
        }
    }
}

Node *relational()
{
    Node *node = add();
    for (;;)
    {
        if (consume("<="))
        {
            node = new_node(ND_LEQ, node, add());
        }
        else if (consume(">="))
        {
            node = new_node(ND_LEQ, add(), node);
        }
        else if (consume("<"))
        {
            node = new_node(ND_LT, node, add());
        }
        else if (consume(">"))
        {
            node = new_node(ND_LT, add(), node);
        }
        else
        {
            return node;
        }
    }
}

Node *add()
{
    Node *node = mul();
    for (;;)
    {
        if (consume("+"))
        {
            node = new_node(ND_ADD, node, mul());
        }
        else if (consume("-"))
        {
            node = new_node(ND_SUB, node, mul());
        }
        else
        {
            return node;
        }
    }
}

Node *mul()
{

    Node *node = unary();
    for (;;)
    {
        if (consume("*"))
        {
            node = new_node(ND_MUL, node, unary());
        }
        else if (consume("/"))
        {
            node = new_node(ND_DIV, node, unary());
        }
        else if (consume("%"))
        {
            node = new_node(ND_MOD, node, unary());
        }
        else
        {
            return node;
        }
    }
}

Node *unary()
{
    if (consume("+"))
    {
        return term();
    }
    if (consume("-"))
    {
        return new_node(ND_SUB, new_node_num(0), term());
    }
    if (consume("&"))
    {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_ADDR;
        node->rhs = unary();
        node->type = calloc(1, sizeof(Type));
        node->type->ty = PTR;
        node->type->ptr_to = node->rhs->type;
        return node;
    }

    if (consume("*"))
    {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_DEREF;
        node->rhs = unary();
        //node->type = calloc(1, sizeof(Type));
        node->type = node->rhs->type->ptr_to;
        return node;
    }
    if (consume_tokenkind(TK_SIZEOF))
    {
        Node *node = unary();
        return new_node_num(type_size(node->type));
    }
    return term();
}

Node *term()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }
    Token *tok = consume_ident();
    if (tok)
    {
        if (consume("("))
        {
            Node *node = calloc(1, sizeof(Node));
            node->args = calloc(1, sizeof(Arg));
            node->kind = ND_CALL;
            node->type = calloc(1, sizeof(Type));
            node->type->ty = INT;
            node->funcname = tok->str;
            node->funcnamelen = tok->len;
            Arg *curarg = node->args;
            if (!check_token(")"))
            {
                for (;;)
                {

                    curarg->node = expr();
                    //curarg->node->kind = ND_NUM;
                    if (!consume(","))
                        break;
                    curarg = new_arg(curarg);
                }
                curarg->next = NULL;
            }
            else
            {
                node->args = NULL;
            }
            expect(")");

            return node;
        }
        else
        {
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_LVAR;
            LVar *lvar = find_lvar(tok, locals);
            if (lvar == NULL)
            {
                node->kind = ND_GVAR;
                lvar = find_lvar(tok, globals);
            }
            if (lvar)
            {

                if (lvar->type->ty == ARRAY)
                {
                    if (consume("["))
                    {
                        node->kind = ND_DEREF;

                        Node *node_lvar = calloc(1, sizeof(Node));
                        node_lvar->kind = ND_LVAR;
                        node_lvar->offset = lvar->offset;
                        node_lvar->type = calloc(1, sizeof(Type));
                        node_lvar->type->ty = PTR;
                        node_lvar->type->ptr_to = lvar->type->ptr_to;

                        Node *node_addr = calloc(1, sizeof(Node));
                        node_addr->kind = ND_ADDR;
                        node_addr->rhs = node_lvar;
                        node_addr->type = calloc(1, sizeof(Type));
                        node_addr->type->ty = PTR;
                        node_addr->type->ptr_to = node_lvar->type->ptr_to;

                        Node *node_index = add();
                        node->rhs = new_node(ND_ADD, node_addr, node_index);
                        node->type = node->rhs->type->ptr_to;
                        expect("]");
                    }
                    else
                    {
                        node->kind = ND_ADDR;
                        Node *node_lvar = calloc(1, sizeof(Node));
                        node_lvar->kind = ND_LVAR;
                        node_lvar->offset = lvar->offset;
                        node_lvar->type = lvar->type;

                        node->rhs = node_lvar;
                        node->type = calloc(1, sizeof(Type));
                        node->type->ty = PTR;
                        node->type->ptr_to = lvar->type->ptr_to;
                    }
                }
                else
                {
                    node->offset = lvar->offset;
                    node->type = lvar->type;
                }
            }
            else
            {
                char *name = calloc(128, sizeof(char));
                strncpy(name, tok->str, tok->len);
                error("定義されていない変数名です:%s\n", name);
            }
            return node;
        }
    }
    return new_node_num(expect_number());
}
