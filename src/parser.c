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

        // 1. 'Num' அல்லது 'முழுஎண்' (Variable Declaration & Addition)
        if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "முழுஎண்") == 0) {
            Token name_token = get_next_token(file); // மாறியின் பெயர் (p, c, etc.)
            
            // '=' வரும் வரை தேடுவோம்
            while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); 

            Token first_val = get_next_token(file); // முதல் மதிப்பு அல்லது மாறி

            // --- கூட்டல் லாஜிக் செக் (Advanced) ---
            Token op = get_next_token(file);
            if (op.type == 19 || strcmp(op.value, "+") == 0) {
                Token second_val = get_next_token(file);
                // codegen-ல் நாம் உருவாக்கிய gen_var_add-ஐ கூப்பிடுவோம்
                tamizhi_gen_var_add(name_token.value, first_val.value, second_val.value);
            } else {
                // சாதாரண அசைன்மென்ட் (Num p = 100)
                if (isdigit(first_val.value[0])) {
                    tamizhi_gen_var(name_token.value, atoi(first_val.value));
                }
                // 'op' டோக்கன் ஒருவேளை அடுத்த வரியோடதா இருக்கலாம், அதனால் அதை கவனிக்கணும்
                // இப்போதைக்கு சிம்பிளா விட்டுடுவோம்
            }
            continue;
        }

        // 2. 'அச்சிடு' (The Working Fix)
        else if (t.type == T_PRINT || strcmp(t.value, "அச்சிடு") == 0) {
            t = get_next_token(file);
            
            if (t.type == 15 || strcmp(t.value, "(") == 0) { 
                t = get_next_token(file);
            }
            
            fprintf(stderr, "[Parser] Printing Variable: %s\n", t.value);
            tamizhi_gen_print(t.value);

            // எக்ஸ்ட்ரா டோக்கன்களை எடுக்க வேண்டாம், அது அடுத்த வரியை பாதிக்கும்.
            continue;
        }

        // 3. 'சு' (Loop)
        else if (t.type == T_FOR || strcmp(t.value, "சு") == 0) {
            while ((t = get_next_token(file)).type != T_NUM && t.type != T_EOF);
            if (t.type == T_NUM) {
                tamizhi_gen_loop_test(atoi(t.value));
            }
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF);
            skip_block(file);
            continue;
        }

        // 4. 'முதன்மை' அல்லது மத்த பிளாக்குகளைத் தாண்டுதல்
        else if (t.type == T_MAIN || t.type == T_FUNC || t.type == 22) {
            skip_block(file);
            continue;
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
