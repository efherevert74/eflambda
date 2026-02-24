#include <ctype.h>
#include <math.h>
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
    PVar,
    PAbs,
    PApp,
    PInv,
} TermType;

struct Term {
    TermType type;
    union {
        Var var;
        Abs abs;
        App app;
    };
};

Term *parse(char **str) {
    Tok tok = lex(str);
    Term *term = malloc(sizeof(Term));
    switch (tok.type) {
    case LVar:
        term->type = PVar;
        term->var = (Var){tok.var};
        break;
    case LLambda:
        term->type = PAbs;

        Term *var = parse(str);
        if (var->type != PVar || lex(str).type != LDot) {
            printf("a");
            term->type = PInv;
            break;
        }
        Term *body = parse(str);

        term->abs = (Abs){var->var, body};
        break;
    case LLParen:
        term->type = PApp;

        Term *left = parse(str);
        char *str_lookahead = *str;
        // abstraction/variable
        if (lex(&str_lookahead).type == LRParen) {
            *term = *left;
            *str = str_lookahead;
            break;
        }
        // application
        Term *right = parse(str);
        if (lex(str).type != LRParen) {
            term->type = PInv;
            printf("b");
        }

        term->app = (App){left, right};
        break;
    case LRParen:
    case LDot:
    case LInv:
    case LEof:
        printf("c");
        term->type = PInv;
        break;
    }

    return term;
}

int display(char *buf, int buf_len, Term *term) {
    int n = 0;
    switch (term->type) {
    case PVar:
        n += snprintf(buf, buf_len, "%c", term->var.name);
        break;
    case PAbs:
        n += snprintf(buf, buf_len, "\\%c.", term->abs.var.name);
        n += display(buf + n, buf_len - n, term->abs.body);

        break;
    case PApp:
        n += snprintf(buf, buf_len, "(");
        n += display(buf + n, buf_len - n, term->app.left);
        n += snprintf(buf + n, buf_len - n, " ");
        n += display(buf + n, buf_len - n, term->app.right);
        n += snprintf(buf + n, buf_len - n, ")");

        break;
    case PInv:
        break;
    }
    buf[n] = '\0';
    return n;
}

void subst(Term *in, Var what, Term *to) {
    switch (in->type) {
    case PVar:
        if (in->var.name == what.name) {
            *in = *to;
        }
        break;
    case PAbs:
        subst(in->abs.body, what, to);
        break;
    case PApp:
        subst(in->app.left, what, to);
        subst(in->app.right, what, to);
        break;
    case PInv:
        break;
    }
}

bool reduce(Term *term) {
    if (term->type == PApp) {
        Term *left = term->app.left;
        Term *right = term->app.right;
        bool reduced = reduce(left) || reduce(right);

        if (left->type == PAbs) {
            subst(left, left->abs.var, right);
            *term = *left->abs.body;
            return true;
        }
        return reduced;
    } else if (term->type == PAbs) {
        return reduce(term->abs.body);
    }
    return false;
}
