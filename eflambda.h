#ifndef EFLAMBDA_H
#define EFLAMBDA_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef enum {
    LLParen,
    LRParen,
    LDot,
    LLambda,
    LVar,
    LEq,
    LInv,
    LEof,
} TokType;

typedef struct {
    TokType type;
    char *var;
} Tok;

bool is_valid_for_var(char c) {
    return !isspace(c) && c != '(' && c != ')' && c != '.' && c != '\\' &&
           c != '=';
}

Tok lex(char **str) {
    char c;
    Tok tok;

    while ((c = *((*str)++)) != '\0') {
        switch (c) {
        case ' ':
        case '\n':
        case '\t':
            continue;
        case '(':
            tok.type = LLParen;
            break;
        case ')':
            tok.type = LRParen;
            break;
        case '.':
            tok.type = LDot;
            break;
        case '\\':
            tok.type = LLambda;
            break;
        case '=':
            tok.type = LEq;
            break;
        default:
            (*str)--;
            int n = 0;
            char c;
            while ((c = (*str)[n]) && is_valid_for_var(c)) {
                n++;
            }
            if (n == 0) {
                tok.type = LInv;
                return tok;
            }

            tok.type = LVar;
            tok.var = memcpy(malloc(n + 1), *str, n);
            tok.var[n] = '\0';
            *str += n;

            break;
        }
        return tok;
    }

    tok.type = LEof;
    return tok;
}

// lambda ::= "\"
//
// term ::= variable | abstraction | application
// variable ::= [a-zA-Z]+
// abstraction ::= lambda variable "." term | "(" abstraction ")"
// application ::= term term

typedef struct Term Term;

typedef struct {
    char *name;
} Var;

typedef struct {
    Var var;
    Term *body;
} Abs;

typedef struct {
    Term *left;
    Term *right;
} App;

typedef enum {
    TVar,
    TAbs,
    TApp,
    TInv,
} TermType;

struct Term {
    TermType type;
    union {
        Var var;
        Abs abs;
        App app;
    };
};

typedef struct {
    char *key;
    Term value;
} VarLib;

// Function headers
Term *term_parse(char **str, VarLib **lib, bool read_only);
Term *term_parse_once(char **str, VarLib **lib, bool read_only);

void term_subst(Term *in, Var what, Term *to);
bool term_reduce(Term *term, VarLib **lib, bool lazy);

Term *term_copy(Term *term);
void term_free(Term *term);

int term_display(char *buf, int buf_len, Term *term);
void term_dbg(Term *term);

Term *term_parse_once(char **str, VarLib **lib, bool read_only) {
    Tok tok = lex(str);
    Term *term = malloc(sizeof(Term));

    switch (tok.type) {
    case LVar:
        term->type = TVar;
        term->var = (Var){tok.var};
        break;
    case LLambda: {
        term->type = TAbs;

        Term *var = term_parse_once(str, lib, read_only);
        if (var->type != TVar) {
            term_free(var);
            term->type = TInv;
            break;
        }

        Tok dot = lex(str);
        if (dot.type != LDot) {
            term_free(var);
            if (dot.type == LVar) {
                free(dot.var);
            }
            term->type = TInv;
            break;
        }
        Term *body = term_parse_once(str, lib, read_only);
        if (body->type == TInv) {
            term_free(var);
            term_free(body);
            term->type = TInv;
            break;
        }

        term->abs = (Abs){var->var, body};
        free(var);
        break;
    }
    case LLParen:
        free(term);
        term = term_parse(str, lib, read_only);
        break;
    case LEq:
        free(term);
        term = term_parse(str, lib, read_only);
        break;
    case LRParen:
    case LDot:
    case LInv:
    case LEof:
        term->type = TInv;
        break;
    }

    return term;
}

Term *term_parse(char **str, VarLib **lib, bool read_only) {
    Term *term = term_parse_once(str, lib, read_only);
    if (term->type == TInv) {
        return term;
    }

    while (true) {
        if (term->type == TVar) {
            char *str_lookahead = *str;
            Tok tok = lex(&str_lookahead);
            if (tok.type == LEq) {
                *str = str_lookahead;
                Term *right = term_parse(str, lib, read_only);
                if (!read_only) {
                    Term *value_copy = term_copy(right);
                    shput(*lib, term->var.name, *value_copy);
                    free(value_copy);
                }
                free(term);
                return right;
            } else if (tok.type == LVar) {
                free(tok.var); // free the lookahead name
            }
        }

        Term *right = term_parse_once(str, lib, read_only);
        if (right->type == TInv) {
            term_free(right);
            break;
        }

        Term *left = term;
        term = malloc(sizeof(Term));
        term->type = TApp;
        term->app.right = right;
        term->app.left = left;
    }
    return term;
}

Term *term_copy(Term *term) {
    Term *copy = malloc(sizeof(Term));
    copy->type = term->type;
    switch (term->type) {
    case TVar:
        copy->var.name = strdup(term->var.name);
        break;
    case TAbs:
        copy->abs.var.name = strdup(term->abs.var.name);
        copy->abs.body = term_copy(term->abs.body);
        break;
    case TApp:
        copy->app.left = term_copy(term->app.left);
        copy->app.right = term_copy(term->app.right);
        break;
    case TInv:
        break;
    }
    return copy;
}

bool occurs_free(Term *term, Var var) {
    switch (term->type) {
    case TVar:
        return strcmp(term->var.name, var.name) == 0;
    case TAbs:
        // don't count occurrences bound by this lambda
        if (strcmp(term->abs.var.name, var.name) == 0) {
            return false;
        }
        return occurs_free(term->abs.body, var);
    case TApp:
        return occurs_free(term->app.left, var) ||
               occurs_free(term->app.right, var);
    default:
        return false;
    }
}

void rename_var(Term *in, Var what, Var to) {
    switch (in->type) {
    case TVar:
        if (strcmp(in->var.name, what.name) == 0) {
            free(in->var.name);
            in->var.name = strdup(to.name);
        }
        break;
    case TAbs:
        if (strcmp(in->abs.var.name, what.name) != 0) {
            rename_var(in->abs.body, what, to);
        }
        break;
    case TApp:
        rename_var(in->app.left, what, to);
        rename_var(in->app.right, what, to);
        break;
    case TInv:
        break;
    }
}

void term_subst(Term *in, Var what, Term *to) {
    switch (in->type) {
    case TVar:
        if (strcmp(in->var.name, what.name) == 0) {
            Term *copy = term_copy(to);
            char *old_name = in->var.name;
            *in = *copy;
            free(copy);
            free(old_name);
        }
        break;
    case TAbs:
        // don't count occurrences bound by this lambda
        if (strcmp(in->abs.var.name, what.name) == 0) {
            break;
        }

        if (occurs_free(to, in->abs.var)) {
            // rename via alpha-equivalence
            static int name_idx = 0;

            int n = snprintf(NULL, 0, "%s%i", in->abs.var.name, name_idx) + 1;

            char *new_name = malloc(n);
            snprintf(new_name, n, "%s%i", in->abs.var.name, name_idx);

            Var new_var = {new_name};

            rename_var(in->abs.body, in->abs.var, new_var);
            free(in->abs.var.name);
            in->abs.var = new_var;
        }

        term_subst(in->abs.body, what, to);
        break;
    case TApp:
        term_subst(in->app.left, what, to);
        term_subst(in->app.right, what, to);
        break;
    case TInv:
        break;
    }
}

typedef struct BoundVarCtx BoundVarCtx;
struct BoundVarCtx {
    const char *name;
    BoundVarCtx *parent;
};

bool term_reduce_worker(Term *term, VarLib **lib, BoundVarCtx *ctx, bool lazy) {
    if (term->type == TApp) {
        Term *left = term->app.left;
        Term *right = term->app.right;

        // beta-reduction
        if (left->type == TAbs) {
            term_subst(left->abs.body, left->abs.var, right);

            Term *body = left->abs.body;
            *term = *body;

            free(body);
            free(left->abs.var.name);
            free(left);
            term_free(right);

            return true;
        }

        return term_reduce_worker(left, lib, ctx, lazy) ||
               term_reduce_worker(right, lib, ctx, lazy);

    } else if (term->type == TAbs) {
        if (lazy) {
            return false;
        }
        BoundVarCtx new_ctx = {term->abs.var.name, ctx};
        return term_reduce_worker(term->abs.body, lib, &new_ctx, lazy);
    } else if (term->type == TVar) {
        for (BoundVarCtx *cur = ctx; cur != NULL; cur = cur->parent) {
            if (strcmp(cur->name, term->var.name) == 0) {
                return false;
            }
        }
        int idx = shgeti(*lib, term->var.name);
        if (idx != -1) {
            Term *copy = term_copy(&(*lib)[idx].value);
            char *old_name = term->var.name;
            *term = *copy;
            free(copy);
            free(old_name);
            return true;
        }
        return false;
    }
    return false;
}

bool term_reduce(Term *term, VarLib **lib, bool lazy) {
    return term_reduce_worker(term, lib, NULL, lazy);
}

void term_free(Term *term) {
    if (!term)
        return;
    switch (term->type) {
    case TVar:
        free(term->var.name);
        break;
    case TAbs:
        free(term->abs.var.name);
        term_free(term->abs.body);
        break;
    case TApp:
        term_free(term->app.left);
        term_free(term->app.right);
        break;
    case TInv:
        break;
    }
    free(term);
}

int term_display(char *buf, int buf_len, Term *term) {
    int n = 0;
    switch (term->type) {
    case TVar:
        n += snprintf(buf, buf_len, "%s", term->var.name);
        break;
    case TAbs:
        n += snprintf(buf, buf_len, "\\%s.", term->abs.var.name);
        if (buf == NULL && buf_len == 0) {
            n += term_display(NULL, 0, term->abs.body);
        } else {
            n += term_display(buf + n, buf_len - n, term->abs.body);
        }

        break;
    case TApp:
        if (buf == NULL && buf_len == 0) {
            n += term_display(NULL, 0, term->app.left);
            n += term_display(NULL, 0, term->app.right);
            n += 3;
        } else {
            n += snprintf(buf, buf_len, "(");
            n += term_display(buf + n, buf_len - n, term->app.left);
            n += snprintf(buf + n, buf_len - n, " ");
            n += term_display(buf + n, buf_len - n, term->app.right);
            n += snprintf(buf + n, buf_len - n, ")");
        }

        break;
    case TInv:
        n += snprintf(buf, buf_len, "INV");
        break;
    }
    if (buf == NULL) {
        n += 1;
    }
    return n;
}

void term_dbg(Term *term) {
    char buf[256];
    term_display(buf, sizeof(buf), term);
    char *typ;
    switch (term->type) {
    case TVar:
        typ = "TVar";
        break;
    case TAbs:
        typ = "TAbs";
        break;
    case TApp:
        typ = "TApp";
        break;
    case TInv:
        typ = "TInv";
        break;
    }
    printf("%s:\t%s\n", typ, buf);
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !EFLAMBDA_H
