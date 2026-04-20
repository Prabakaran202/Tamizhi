#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Helper to skip blocks { ... }
void skip_block(FILE *file) {
    Token t;
    int brace_count = 1;
    while (brace_count > 0 && (t = get_next_token(file)).type != T_EOF) {
        if (t.type == 22) brace_count++; // {
        if (t.type == 23) brace_count--; // }
    }
}

void parse(FILE *file) {
    Token t;
    fprintf(stderr, "[Parser] Tamizhi Engine Analysis Started...\n");

    while ((t = get_next_token(file)).type != T_EOF) {
        
        // 1. 'முதன்மை' (Main Block) - It's just a setup, so we skip its body for now
        if (t.type == T_MAIN) {
            fprintf(stderr, "[Parser] Main block detected.\n");
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); // Look for {
            skip_block(file);
        }

        // 2. 'நிகழ்' (Function Definition)
        else if (t.type == T_FUNC) {
            Token name = get_next_token(file);
            fprintf(stderr, "[Parser] Function definition found: %s\n", name.value);
            // Skip until semicolon or body
            while ((t = get_next_token(file)).type != 17 && t.type != 22 && t.type != T_EOF);
            if (t.type == 22) skip_block(file);
        }

        // 3. 'இயக்கு' (The Execution logic)
        else if (t.type == T_CALL) {
            fprintf(stderr, "[Parser] Starting Execution Block...\n");
            
            while ((t = get_next_token(file)).type != 23 && t.type != T_EOF) {
                // If we find the function call inside 'இயக்கு'
                if (t.type == T_ID) {
                    char func_name[64];
                    strcpy(func_name, t.value);

                    // Look for numbers inside brackets: add(10+20)
                    while ((t = get_next_token(file)).type != T_NUM && t.type != 23 && t.type != T_EOF);
                    
                    if (t.type == T_NUM) {
                        int n1 = atoi(t.value);
                        get_next_token(file); // Skip '+'
                        Token second = get_next_token(file);
                        int n2 = atoi(second.value);

                        fprintf(stderr, "[Parser] Executing %s with: %d + %d\n", func_name, n1, n2);
                        
                        // 🔥 TRIGGER BACKEND
                        tamizhi_gen_add_and_print(n1, n2);
                    }
                }
            }
        }
    }
}
