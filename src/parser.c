#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// 1. Prototype
void skip_block(FILE *file); 

void parse(FILE *file) {
    Token t;
    fprintf(stderr, "[Parser] Tamizhi Engine Analysis Started...\n");

    while ((t = get_next_token(file)).type != T_EOF) {

        // --- 'முதன்மை' (Main Block) ---
        if (t.type == T_MAIN) {
            fprintf(stderr, "[Parser] Main block detected.\n");
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); 
            skip_block(file);
            continue;
        }

        // --- 'நிகழ்' (Function Definition) ---
        else if (t.type == T_FUNC) {
            fprintf(stderr, "[Parser] Function definition detected.\n");
            while ((t = get_next_token(file)).type != 17 && t.type != 22 && t.type != T_EOF);
            if (t.type == 22) skip_block(file);
            continue;
        }

        // --- 'இயக்கு' (Execution Block) ---
        else if (t.type == T_CALL || strcmp(t.value, "இயக்கு") == 0) {
            fprintf(stderr, "[Parser] Starting Execution (இயக்கு)...\n");

            // '}' வரும் வரை உள்ளே இருக்கும் எண்களைத் தேடுவோம்
            while ((t = get_next_token(file)).type != 23 && t.type != T_EOF) {
                
                // 1. முதல் எண்ணைக் கண்டுபிடி (T_NUM)
                if (t.type == T_NUM) {
                    int n1 = atoi(t.value); 

                    // 2. அடுத்த எண்ணைக் கண்டுபிடிக்கும் வரை இடையில் இருப்பவற்றை (add, +, () தாண்டுவோம்
                    while ((t = get_next_token(file)).type != T_NUM && t.type != 23 && t.type != T_EOF);

                    if (t.type == T_NUM) {
                        int n2 = atoi(t.value);
                        fprintf(stderr, "[Parser] Logic Found: %d + %d\n", n1, n2);
                        
                        // Backend-ஐ அழைத்து அவுட்புட் உருவாக்குதல்
                        tamizhi_gen_add_and_print(n1, n2);
                        break; 
                    }
                }
            }
        }
    }
}

// 2. skip_block - இது பிளாக்குகளை சரியாக மூடும் வரை தாண்டும்
void skip_block(FILE *file) {
    Token t;
    int brace_count = 1;
    while (brace_count > 0 && (t = get_next_token(file)).type != T_EOF) {
        if (t.type == 22) brace_count++; // '{'
        if (t.type == 23) brace_count--; // '}'
    }
}
