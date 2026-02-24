#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>

#include "lambda.c"

#define MAX_INPUT_LEN 1024

int main(int argc, char **argv) {
    char inp_buf[MAX_INPUT_LEN];
    while (true) {
        printf("$ ");
        fgets(inp_buf, MAX_INPUT_LEN, stdin);

        if (inp_buf[0] == ':' || inp_buf[0] == ';') {
            if (inp_buf[1] == 'q') {
                printf("Exiting.\n");
                exit(0);
            }
        }

        printf("\nInput:  %s\n", inp_buf);

        char *str = inp_buf;
        Tok tok;
        do {
            tok = lex(&str);

            printf("tok.type = %i", tok.type);
            if (tok.type == LVar) {
                printf("\t tok.var = %c", tok.var);
            }
            printf("\n");
        } while (tok.type != LEof);

        str = inp_buf;
        Term* term = parse(&str);
        printf("\n");
        printf("term.type = %i\n", term->type);

        char buf[1024];
        int n = display(buf, sizeof(buf), term);
        buf[n] = '\0';
        printf("\n%s\n", buf);

        // printf("\n");
        // printf("term.abs.type = %i\n", term->abs.body->type);
    }
}
