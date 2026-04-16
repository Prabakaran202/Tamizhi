#include "parser.h"
#include <stdlib.h>

void parse(FILE *file) {
    Token t = get_next_token(file);

    while (t.type != T_EOF) {
        if (t.type == T_INT) { // 'எ' keyword
            printf("[Parser] Variable Declaration detect panniyaachu!\n");
            
            // Next token identifier-a (name) irukanum
            t = get_next_token(file);
            if (t.type == T_ID) {
                printf("[Parser] Variable Name: %s\n", t.value);
            }
        }
        else if (t.type == T_PRINT) { // 'கூறு' keyword
            printf("[Parser] Print statement detect panniyaachu!\n");
        }
        
        t = get_next_token(file);
    }
}
