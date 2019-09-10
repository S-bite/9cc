#include "9cc.h"

LVar *locals;
LVar *globals;
Str *strings;

LVar *find_lvar(Token *tok, LVar *root)
{
    LVar *var = root;
    for (; var != NULL; var = var->next)
    {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
        {
            return var;
        }
    }
    return NULL;
}

Block *new_block(Block *cur)
{
    Block *new = calloc(1, sizeof(Block));
    cur->next = new;
    return new;
}

Block *next_block(Block *cur)
{
    return cur->next;
}

Arg *new_arg(Arg *cur)
{
    Arg *new = calloc(1, sizeof(Arg));
    cur->next = new;
    return new;
}

Arg *next_arg(Arg *cur)
{
    return cur->next;
}

Str *find_str(char *s, int len)
{
    Str *cur = strings;
    for (; cur != NULL; cur = cur->next)
    {
        if (strncmp(cur->str, s, len) == 0)
        {
            return cur;
        }
    }
    return NULL;
}
