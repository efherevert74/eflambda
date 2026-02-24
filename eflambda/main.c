#include <stdbool.h>
#include <stdio.h>

#include "lambda.c"

#define MAX_INPUT_LEN 1024

int main(int argc, char **argv) {
    char buf[MAX_INPUT_LEN];
    while (true) {
        printf("$ ");
        fgets(buf, sizeof(buf), stdin);

        // special commands
        if (buf[0] == ':' || buf[0] == ';') {
            if (buf[1] == 'q') {
                printf("Exiting.\n");
                return 0;
            }
        }

        char *str = buf;
        Term *term = parse(&str);
        display(buf, sizeof(buf), term);
        printf("> %s\n", buf);

        while (reduce(term)) {
            display(buf, sizeof(buf), term);
            printf("> %s\n", buf);
        }

        printf("\n");
    }
}
