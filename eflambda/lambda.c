#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    LLParen,
    LRParen,
    LDot,
    LLambda,
    LVar,
    LInv,
    LEof,
} TokType;

typedef struct {
    TokType type;
    char var;
} Tok;

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
        default:
            if (isalpha(c)) {
                tok.type = LVar;
                tok.var = c;
            }
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
    char name;
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

Term *parse_once(char **str) {
    Tok tok = lex(str);
    Term *term = malloc(sizeof(Term));
    switch (tok.type) {
    case LVar:
        term->type = TVar;
        term->var = (Var){tok.var};
        break;
    case LLambda:
        term->type = TAbs;

        Term *var = parse_once(str);
        if (var->type != TVar || lex(str).type != LDot) {
            term->type = TInv;
            break;
        }
        Term *body = parse_once(str);

        term->abs = (Abs){var->var, body};
        break;
    case LLParen:
        term->type = TApp;

        Term *left = parse_once(str);
        char *str_lookahead = *str;
        // abstraction/variable
        if (lex(&str_lookahead).type == LRParen) {
            *term = *left;
            *str = str_lookahead;
            break;
        }
        // application
        Term *right = parse_once(str);
        if (lex(str).type != LRParen) {
            term->type = TInv;
        }

        term->app = (App){left, right};
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

Term *parse(char **str) {
    Term *term = parse_once(str);
    if (term->type == TInv) {
        return term;
    }
    while (**str != '\0') {
        Term *right = parse_once(str);
        if (right->type != TInv) {
            Term *left = term;
            term = malloc(sizeof(Term));
            term->type = TApp;
            term->app.right = right;
            term->app.left = left;
        }
        else {
            break;
        }
    }
    return term;
}

void subst(Term *in, Var what, Term *to) {
    switch (in->type) {
    case TVar:
        if (in->var.name == what.name) {
            *in = *to;
        }
        break;
    case TAbs:
        subst(in->abs.body, what, to);
        break;
    case TApp:
        subst(in->app.left, what, to);
        subst(in->app.right, what, to);
        break;
    case TInv:
        break;
    }
}

bool reduce(Term *term) {
    if (term->type == TApp) {
        Term *left = term->app.left;
        Term *right = term->app.right;
        bool reduced = reduce(left) || reduce(right);

        if (left->type == TAbs) {
            subst(left, left->abs.var, right);
            *term = *left->abs.body;
            return true;
        }
        return reduced;
    } else if (term->type == TAbs) {
        return reduce(term->abs.body);
    }
    return false;
}

int display(char *buf, int buf_len, Term *term) {
    int n = 0;
    switch (term->type) {
    case TVar:
        n += snprintf(buf, buf_len, "%c", term->var.name);
        break;
    case TAbs:
        n += snprintf(buf, buf_len, "\\%c.", term->abs.var.name);
        n += display(buf + n, buf_len - n, term->abs.body);

        break;
    case TApp:
        n += snprintf(buf, buf_len, "(");
        n += display(buf + n, buf_len - n, term->app.left);
        n += snprintf(buf + n, buf_len - n, " ");
        n += display(buf + n, buf_len - n, term->app.right);
        n += snprintf(buf + n, buf_len - n, ")");

        break;
    case TInv:
        break;
    }
    buf[n] = '\0';
    return n;
}

