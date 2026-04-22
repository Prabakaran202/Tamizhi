#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void skip_block(FILE *file); 

void parse(FILE *file) {
    Token t;
    fprintf(stderr, "[Parser] Tamizhi Engine Analysis Started...\n");

    while ((t = get_next_token(file)).type != T_EOF) {

        // --- 1. 'Num' அல்லது 'முழுஎண்' (Variable & Addition) ---
        if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "முழுஎண்") == 0) {
            Token name_token = get_next_token(file); 
            
            // '=' வரும் வரை தேடுவோம்
            while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); 

            Token first_val = get_next_token(file); 
            
            // அடுத்த டோக்கனைப் பார்ப்போம் (செமிகோலன் அல்லது பிளஸ்)
            Token next_t = get_next_token(file);
            
            if (next_t.type == 19 || strcmp(next_t.value, "+") == 0) {
                // இது கூட்டல் லாஜிக்: Num c = a + b
                Token second_val = get_next_token(file);
                tamizhi_gen_var_add(name_token.value, first_val.value, second_val.value);
            } 
            else {
                // இது சாதாரண அசைன்மென்ட்: Num p = 100
                if (isdigit(first_val.value[0])) {
                    tamizhi_gen_var(name_token.value, atoi(first_val.value));
                }
                
                // மிக முக்கியம்: ஒருவேளை அடுத்த டோக்கன் 'அச்சிடு' ஆக இருந்தால் 
                // அதை இங்கேயே கையாளுவோம் (Token loss தவிர்க்க)
                if (next_t.type == T_PRINT || strcmp(next_t.value, "அச்சிடு") == 0) {
                    Token var_t = get_next_token(file);
                    if (var_t.type == 15) var_t = get_next_token(file); // '(' தாண்டு
                    fprintf(stderr, "[Parser] Printing (Inline): %s\n", var_t.value);
                    tamizhi_gen_print(var_t.value);
                }
            }
            continue;
        }

        // --- 2. 'அச்சிடு' (Stand-alone Print) ---
        else if (t.type == T_PRINT || strcmp(t.value, "அச்சிடு") == 0) {
            Token var_t = get_next_token(file);
            
            // ஒருவேளை '(' இருந்தால் அதைத் தாண்டுவோம்
            if (var_t.type == 15 || strcmp(var_t.value, "(") == 0) { 
                var_t = get_next_token(file);
            }
            
            fprintf(stderr, "[Parser] Printing Variable: %s\n", var_t.value);
            tamizhi_gen_print(var_t.value);
            continue;
        }

        // --- 3. 'சு' (Basic Loop) ---
        else if (t.type == T_FOR || strcmp(t.value, "சு") == 0) {
            while ((t = get_next_token(file)).type != T_NUM && t.type != T_EOF);
            if (t.type == T_NUM) {
                tamizhi_gen_loop_test(atoi(t.value));
            }
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF);
            skip_block(file);
            continue;
        }

        // --- 4. பிளாக்குகளைத் தாண்டுதல் (Main, Func etc.) ---
        else if (t.type == T_MAIN || t.type == T_FUNC || t.type == 22) {
            skip_block(file);
            continue;
        }
    }
}

// பிராக்கெட் பிளாக்குகளைத் தாண்ட உதவும் ஃபங்க்ஷன்
void skip_block(FILE *file) {
    Token t;
    int brace_count = 1;
    while (brace_count > 0 && (t = get_next_token(file)).type != T_EOF) {
        if (t.type == 22) brace_count++; 
        if (t.type == 23) brace_count--; 
    }
}
