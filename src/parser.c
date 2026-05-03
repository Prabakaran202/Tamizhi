#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// பங்க்ஷன் அறிவிப்புகள்
void parse_statement(FILE *file, Token t);
void scan_headers(FILE *file);
void execute_footer(FILE *file);

// 1. மெயின் பார்ஸர் (Advanced 3-Pass System)
void parse(FILE *file) {
    Token t;
    long start_pos = ftell(file);

    fprintf(stderr, "\n[Parser] --- Tamizhi Engine: Advanced 3-Pass Analysis Started ---\n");

    // PASS 1: Header - வரி எண்களை ஸ்கேன் செய்ய
    fprintf(stderr, " -> Phase 1 [Header]: Registering blueprints & line maps...\n");
    scan_headers(file);

    // PASS 2: Body - கோடை மெஷின் லாஜிக்காக மாற்ற
    fseek(file, start_pos, SEEK_SET);
    fprintf(stderr, " -> Phase 2 [Body]: Building core logic engine...\n");
    while ((t = get_next_token(file)).type != T_EOF) {
        if (strcmp(t.value, "call") == 0 || strcmp(t.value, "footer") == 0) break;
        parse_statement(file, t);
    }

    // PASS 3: Footer - ரன் டைம் இயக்கம்
    fprintf(stderr, " -> Phase 3 [Footer]: Launching execution...\n");
    execute_footer(file);

    fprintf(stderr, "[Parser] --- Analysis Completed Successfully ---\n\n");
}

// 2. ஸ்டேட்மென்ட் இன்ஜின் (இதுதான் இப்போ பிக்ஸ் பண்ணப்பட்டது)
void parse_statement(FILE *file, Token t) {
    
    // --- மாறிகள் & கூட்டல் (Variables & Math) ---
    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0 || strcmp(t.value, "முழுஎண்") == 0) {
        Token name_token = get_next_token(file); 
        while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); 
        Token first_val = get_next_token(file); 
        Token next_t = get_next_token(file);

        if (next_t.type == 19 || strcmp(next_t.value, "+") == 0) {
            Token second_val = get_next_token(file);
            tamizhi_gen_var_add(name_token.value, first_val.value, second_val.value);
        } else {
            if (isdigit(first_val.value[0])) {
                tamizhi_gen_var(name_token.value, atoi(first_val.value));
            }
        }
    }

    // --- அச்சிடு (Print) ---
    else if (t.type == T_PRINT || strcmp(t.value, "print") == 0 || strcmp(t.value, "அச்சிடு") == 0) {
        Token var_t = get_next_token(file);
        if (var_t.type == 15) var_t = get_next_token(file); 
        tamizhi_gen_print(var_t.value);
    }

    // --- எனில் & இல்லையெனில் (If-Else - FIXED) ---
    else if (t.type == T_IF || strcmp(t.value, "if") == 0 || strcmp(t.value, "எனில்") == 0) {
        while ((t = get_next_token(file)).type != 15 && t.type != T_EOF); 
        Token var1 = get_next_token(file);
        Token op = get_next_token(file);
        Token var2 = get_next_token(file);

        tamizhi_gen_if_start(var1.value, op.value, var2.value);

        while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); 
        while ((t = get_next_token(file)).type != 23 && t.type != T_EOF) {
            parse_statement(file, t);
        }

        // 'else' செக் - இங்கேதான் தப்பு இருந்தது, இப்போ சரி பண்ணியாச்சு
        Token next_look = get_next_token(file);
        if (strcmp(next_look.value, "else") == 0 || strcmp(next_look.value, "இல்லையெனில்") == 0) {
            tamizhi_gen_else_start();
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); 
            while ((t = get_next_token(file)).type != 23 && t.type != T_EOF) {
                parse_statement(file, t);
            }
        }
        tamizhi_gen_if_end();
    }

    // --- சுற்று (Loop) ---
    else if (t.type == T_FOR || strcmp(t.value, "for") == 0 || strcmp(t.value, "சு") == 0) {
        while ((t = get_next_token(file)).type != T_NUM && t.type != T_EOF);
        int limit = atoi(t.value);
        tamizhi_gen_loop_start(limit);
        while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); 
        while ((t = get_next_token(file)).type != 23 && t.type != T_EOF) {
            parse_statement(file, t);
        }
        tamizhi_gen_loop_end();
    }
}

// 3. Phase 1: Header Scanner (Line numbers)
void scan_headers(FILE *file) {
    Token t;
    while ((t = get_next_token(file)).type != T_EOF) {
        if (strcmp(t.value, "fun") == 0) {
            Token name = get_next_token(file);
            Token check = get_next_token(file);
            if (strcmp(check.value, "line") == 0 || strcmp(check.value, "-") == 0) {
                if (strcmp(check.value, "-") == 0) get_next_token(file);
                Token line_num = get_next_token(file);
                fprintf(stderr, "    [Header] Function '%s' registered at line %s\n", name.value, line_num.value);
            } else {
                fprintf(stderr, "    [Header] Registered: %s\n", name.value);
            }
        }
    }
}

// 4. Phase 3: Footer Runtime
void execute_footer(FILE *file) {
    fprintf(stderr, "    [Runtime] Booting Tamizhi execution environment...\n");
}
