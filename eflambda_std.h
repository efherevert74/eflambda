#ifndef EFLAMBDA_STD_H
#define EFLAMBDA_STD_H

#include "eflambda.h"

void fill_std_lib(VarLib **lib) {
    char *str;
    Term *stdterm;

    // general
    str = "Id = \\x.x";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    // booleans
    str = "True = \\x.\\y.x";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "False = \\x.\\y.y";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "& = \\x.(\\y.x y False)";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "| = \\x.(\\y.x True y)";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "! = \\x.(x False True)";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    // numbers & arithmetic
    str = "0 = False";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "+1 = \\n.\\f.\\x.f (n f x)";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    char buf[16];
    for (int n = 0; n < 10; n++) {
        sprintf(buf, "%i = +1 %i", n + 1, n);
        str = buf;
        stdterm = term_parse(&str, lib, false);
        term_free(stdterm);
    }

    str = "+ = \\m.\\n.m +1 n";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "* = \\m.\\n.\\f.\\x.m (n f) x";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "-1 = \\n.\\f.\\x.n (\\g.\\h.h (g f)) (\\u.x) (\\u.u)";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "- = \\m.\\n.n -1 m";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "0? = \\n.n (\\x.False) True";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "Eq = \\n.\\m.& (0? (- n m)) (0? (- m n))";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    // fixed point combinators
    str = "Y = \\f.(\\x.f (x x))(\\x.f (x x))";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "O = (\\x.\\y.y (x x y)) (\\x.\\y.y (x x y))";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    str = "Z = \\f.(\\x.f (\\v.x x v)) (\\x.f (\\v.x x v))";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);

    // recursion
    str = "Fib = Y \\fib.\\n.0? n 0 "
          "(0? (-1 n) 1 "
          "(+ (fib "
          "(-1 n)) (fib (-1 (-1 n)))))";
    stdterm = term_parse(&str, lib, false);
    term_free(stdterm);
}

#endif // !EFLAMBDA_STD_H
