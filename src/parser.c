#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// 1. Prototype - முன்னாடியே அறிவிக்கிறோம்
void skip_block(FILE *file); 

void parse(FILE *file) {
    Token t;
    fprintf(stderr, "[Parser] Tamizhi Engine Analysis Started...\n");

    while ((t = get_next_token(file)).type != T_EOF) {

        // --- 'முதன்மை' ---
        if (t.type == T_MAIN) {
            fprintf(stderr, "[Parser] Main block detected.\n");
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); 
            skip_block(file); 
        }

        // --- 'நிகழ்' ---
        else if (t.type == T_FUNC) {
            fprintf(stderr, "[Parser] Function definition detected.\n");
            while ((t = get_next_token(file)).type != 17 && t.type != 22 && t.type != T_EOF);
            if (t.type == 22) skip_block(file);
        }

        // --- 'இயக்கு' ---
        else if (t.type == T_CALL || strcmp(t.value, "இயக்கு") == 0) {
            fprintf(stderr, "[Parser] Starting Execution (இயக்கு)...\n");

            while ((t = get_next_token(file)).type != T_ID && t.type != 23 && t.type != T_EOF);

            if (t.type == T_ID) {
                while ((t = get_next_token(file)).type != T_NUM && t.type != 23 && t.type != T_EOF);

                if (t.type == T_NUM) {
                    int n1 = atoi(t.value); 
                    get_next_token(file);   // '+' குறியீடு
                    Token second = get_next_token(file); 
                    int n2 = atoi(second.value);

                    fprintf(stderr, "[Parser] Logic Found: %d + %d\n", n1, n2);
                    tamizhi_gen_add_and_print(n1, n2);
                }
            }
        }
    } // while லூப் இங்க முடியுது
} // parse ஃபங்க்ஷன் இங்க முடியுது

// 2. இப்போ தான் skip_block-ஐ தனியா வெளிய எழுதணும்
void skip_block(FILE *file) {
    Token t;
    int brace_count = 1;
    while (brace_count > 0 && (t = get_next_token(file)).type != T_EOF) {
        if (t.type == 22) brace_count++; 
        if (t.type == 23) brace_count--; 
    }
}
