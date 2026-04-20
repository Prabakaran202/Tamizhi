#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// 1. Prototype - முன்னாடியே அறிவிக்கிறோம் (இதை நீக்காதீங்க)
void skip_block(FILE *file); 

void parse(FILE *file) {
    Token t;
    fprintf(stderr, "[Parser] Tamizhi Engine Analysis Started...\n");

    while ((t = get_next_token(file)).type != T_EOF) {

        // --- 'முதன்மை' (Main Block) ---
        if (t.type == T_MAIN) {
            fprintf(stderr, "[Parser] Main block detected.\n");
            // '{' வரும் வரை தேடு
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); 
            skip_block(file); // அந்த பிளாக்கை அப்படியே தாண்டு
            continue;
        }

        // --- 'நிகழ்' (Function Definition) ---
        else if (t.type == T_FUNC) {
            fprintf(stderr, "[Parser] Function definition detected.\n");
            // ';' அல்லது '{' வரும் வரை தேடு
            while ((t = get_next_token(file)).type != 17 && t.type != 22 && t.type != T_EOF);
            if (t.type == 22) skip_block(file);
            continue;
        }

        // --- 'இயக்கு' (Execution Block) ---
        else if (t.type == T_CALL || strcmp(t.value, "இயக்கு") == 0) {
            fprintf(stderr, "[Parser] Starting Execution (இயக்கு)...\n");

            // இந்த பிளாக்குக்கு உள்ளே ('}' வரும் வரை) எண்களைத் தேடுவோம்
            while ((t = get_next_token(file)).type != 23 && t.type != T_EOF) {
                
                if (t.type == T_NUM) {
                    int n1 = atoi(t.value); // முதல் எண் (10)

                    // அடுத்த டோக்கனை எடுப்போம் (இது '+' ஆக இருக்கணும்)
                    get_next_token(file); 

                    // இரண்டாவது எண் (20)
                    Token second = get_next_token(file);
                    if (second.type == T_NUM) {
                        int n2 = atoi(second.value);
                        fprintf(stderr, "[Parser] Logic Found: %d + %d\n", n1, n2);
                        
                        // Backend-க்கு தகவல் அனுப்பி LLVM IR-ஐ உருவாக்குதல்
                        tamizhi_gen_add_and_print(n1, n2);
                        break; // கணக்கீடு முடிந்ததும் இந்த லூப்பில் இருந்து வெளியே வரலாம்
                    }
                }
            }
        }
    }
}

// 2. skip_block ஃபங்க்ஷன் - இது parse-க்கு வெளியே இருக்கணும்
void skip_block(FILE *file) {
    Token t;
    int brace_count = 1;
    while (brace_count > 0 && (t = get_next_token(file)).type != T_EOF) {
        if (t.type == 22) brace_count++; // '{'
        if (t.type == 23) brace_count--; // '}'
    }
}
