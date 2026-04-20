/*#include "parser.h"
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

        // --- 'சு' (Loop - ஸ்பீடு டெஸ்ட் செய்ய) ---
        else if (t.type == T_FOR || strcmp(t.value, "சு") == 0) {
            fprintf(stderr, "[Parser] Loop detection started...\n");

            // லூப் லிமிட் (எண்) கிடைக்கும் வரை தேடுவோம்
            while ((t = get_next_token(file)).type != T_NUM && t.type != T_EOF);
            
            if (t.type == T_NUM) {
                int limit = atoi(t.value);
                fprintf(stderr, "[Parser] Loop limit set to: %d\n", limit);
                
                // codegen.c-ல் இருக்கும் LLVM Loop லாஜிக்கை இங்க கூப்பிடுறோம்
                tamizhi_gen_loop_test(limit);
            }
            
            // லூப் பாடியைத் தாண்ட (தற்போதைக்கு ஸ்கிப் செய்வோம்)
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF);
            skip_block(file);
            continue;
        }

        // --- 'இயக்கு' (Execution Block) ---
        else if (t.type == T_CALL || strcmp(t.value, "இயக்கு") == 0) {
            fprintf(stderr, "[Parser] Starting Execution (இயக்கு)...\n");

            while (1) {
                t = get_next_token(file);
                if (t.type == 23 || t.type == T_EOF) break; 

                if (t.value[0] >= '0' && t.value[0] <= '9') {
                    int n1 = atoi(t.value);
                    fprintf(stderr, "[Parser] Found First Number: %d\n", n1);

                    while (1) {
                        t = get_next_token(file);
                        if (t.type == 23 || t.type == T_EOF) break;
                        if (t.value[0] >= '0' && t.value[0] <= '9') break;
                    }

                    if (t.value[0] >= '0' && t.value[0] <= '9') {
                        int n2 = atoi(t.value);
                        fprintf(stderr, "[Parser] Found Second Number: %d\n", n2);
                        fprintf(stderr, "[Parser] Logic Found: %d + %d\n", n1, n2);
                        
                        tamizhi_gen_add_and_print(n1, n2);
                        break; 
                    }
                }
            }
        }
    }
}

// 2. skip_block - பிளாக்குகளைத் தாண்ட
void skip_block(FILE *file) {
    Token t;
    int brace_count = 1;
    while (brace_count > 0 && (t = get_next_token(file)).type != T_EOF) {
        if (t.type == 22) brace_count++; // '{'
        if (t.type == 23) brace_count--; // '}'
    }
}
*/

#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

        // --- 'முழுஎண்' (Variable Declaration: முழுஎண் அ = 10) ---
        else if (t.type == T_INT || strcmp(t.value, "முழுஎண்") == 0) {
            fprintf(stderr, "[Parser] Variable declaration detected...\n");
            
            // மாறியின் பெயரை எடுப்போம் (உதாரணம்: 'அ')
            Token name_token = get_next_token(file); 
            
            // '=' வரும் வரை தாண்டுவோம்
            while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); 

            // மதிப்பை எடுப்போம் (உதாரணம்: '10')
            Token val_token = get_next_token(file);
            
            if (val_token.value[0] >= '0' && val_token.value[0] <= '9') {
                int val = atoi(val_token.value);
                // Codegen-க்கு அனுப்புவோம்
                tamizhi_gen_var(name_token.value, val);
            }
            continue;
        }

        // --- 'அச்சிடு' (Print Variable: அச்சிடு அ) ---
        else if (t.type == T_PRINT || strcmp(t.value, "அச்சிடு") == 0) {
            Token var_token = get_next_token(file);
            fprintf(stderr, "[Parser] Print variable: %s\n", var_token.value);
            tamizhi_gen_print(var_token.value);
            continue;
        }

        // --- 'சு' (Loop) ---
        else if (t.type == T_FOR || strcmp(t.value, "சு") == 0) {
            fprintf(stderr, "[Parser] Loop detection started...\n");
            while ((t = get_next_token(file)).type != T_NUM && t.type != T_EOF);
            if (t.type == T_NUM) {
                int limit = atoi(t.value);
                tamizhi_gen_loop_test(limit);
            }
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF);
            skip_block(file);
            continue;
        }

        // --- 'இயக்கு' (Execution Block) ---
        else if (t.type == T_CALL || strcmp(t.value, "இயக்கு") == 0) {
            fprintf(stderr, "[Parser] Starting Execution (இயக்கு)...\n");
            while (1) {
                t = get_next_token(file);
                if (t.type == 23 || t.type == T_EOF) break; 
                if (t.value[0] >= '0' && t.value[0] <= '9') {
                    int n1 = atoi(t.value);
                    while (1) {
                        t = get_next_token(file);
                        if (t.type == 23 || t.type == T_EOF) break;
                        if (t.value[0] >= '0' && t.value[0] <= '9') break;
                    }
                    if (t.value[0] >= '0' && t.value[0] <= '9') {
                        int n2 = atoi(t.value);
                        tamizhi_gen_add_and_print(n1, n2);
                        break; 
                    }
                }
            }
        }
    }
}

void skip_block(FILE *file) {
    Token t;
    int brace_count = 1;
    while (brace_count > 0 && (t = get_next_token(file)).type != T_EOF) {
        if (t.type == 22) brace_count++; 
        if (t.type == 23) brace_count--; 
    }
}
