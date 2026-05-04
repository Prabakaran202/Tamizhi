#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ஹெல்பர் பங்க்ஷன்: டோக்கன் பாதுகாப்பைச் சரிபார்க்க
int is_valid(Token t) {
    if (t.value == NULL || strlen(t.value) == 0) return 0;
    return 1;
}

void parse_statement(FILE *file, Token t);
void scan_headers(FILE *file);
void execute_footer(FILE *file, long start_pos);

// 1. மெயின் பார்ஸர்
void parse(FILE *file) {
    Token t;
    long start_pos = ftell(file);

    fprintf(stderr, "\n[Parser] --- Tamizhi Engine: Advanced 3-Pass Analysis Started ---\n");

    // PASS 1: Header Scanning
    scan_headers(file);

    // PASS 2: Body Mapping
    rewind(file); 
    fprintf(stderr, " -> Phase 2 [Body]: Mapping logic to DNA-VM...\n");
    while ((t = get_next_token(file)).type != T_EOF) {
        if (!is_valid(t)) continue; 
        if (strcmp(t.value, "footer") == 0) break;

        if (strcmp(t.value, "fun") == 0) {
            Token name = get_next_token(file);
            if (!is_valid(name)) break; 
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); 
            while ((t = get_next_token(file)).type != 23 && t.type != T_EOF) {
                if (is_valid(t)) parse_statement(file, t);
            }
            continue;
        }
    }

    // PASS 3: Footer Execution
    rewind(file);
    fprintf(stderr, " -> Phase 3 [Footer]: Launching execution...\n");
    execute_footer(file, 0L);
}

// 2. ஸ்டேட்மென்ட் இன்ஜின் (Expression Support சேர்க்கப்பட்டது)
void parse_statement(FILE *file, Token t) {
    if (!is_valid(t)) return;

    // --- மாறிகள் டிக்ளரேஷன் ---
    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {
        Token name_token = get_next_token(file); 
        while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); // '='

        Token first_val = get_next_token(file); 
        Token next_t = get_next_token(file);

        if (is_valid(next_t) && (next_t.type == 19 || strcmp(next_t.value, "+") == 0)) {
            Token second_val = get_next_token(file);
            tamizhi_gen_var_add(name_token.value, first_val.value, second_val.value);
        } else {
            if (is_valid(first_val) && isdigit(first_val.value[0])) {
                tamizhi_gen_var(name_token.value, atoi(first_val.value));
            }
        }
    }
    // --- மேம்படுத்தப்பட்ட அச்சிடு (Direct Numbers & Expressions) ---
    else if (t.type == T_PRINT || strcmp(t.value, "print") == 0 || strcmp(t.value, "அச்சிடு") == 0) {
        Token first = get_next_token(file);
        
        // அடைப்புக்குறி இருந்தால் அதைத் தாண்டு (எ.கா: print(10))
        if (first.type == 15) first = get_next_token(file);

        long pos = ftell(file);
        Token op = get_next_token(file);

        // 1 + 1 போன்ற ஆபரேஷன் இருக்கிறதா என்று பார்க்க
        if (is_valid(op) && (strcmp(op.value, "+") == 0)) {
            Token second = get_next_token(file);
            // தற்காலிக வேரியபிளில் கூட்டி பின் பிரிண்ட் செய்யவும்
            tamizhi_gen_var_add("temp_res", first.value, second.value);
            tamizhi_gen_print("temp_res");
        } 
        else {
            // வெறும் print a அல்லது print 100 இருந்தால்
            fseek(file, pos, SEEK_SET); // ஆபரேஷன் இல்லையென்றால் பழைய இடத்திற்கே திரும்பு
            if (is_valid(first)) tamizhi_gen_print(first.value);
        }
    }
}

// 3. Phase 1 & 3 லாஜிக் (மாற்றமில்லை)
void scan_headers(FILE *file) {
    Token t;
    while ((t = get_next_token(file)).type != T_EOF) {
        if (is_valid(t) && strcmp(t.value, "fun") == 0) {
            Token name = get_next_token(file);
            if (is_valid(name)) fprintf(stderr, "    [Header] Registered function: %s\n", name.value);
        }
        if (is_valid(t) && strcmp(t.value, "footer") == 0) break; 
    }
}

void execute_footer(FILE *file, long start_pos) {
    Token t;
    int in_footer = 0;
    while ((t = get_next_token(file)).type != T_EOF) {
        if (is_valid(t) && strcmp(t.value, "footer") == 0) {
            in_footer = 1;
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); 
            continue;
        }
        if (in_footer) {
            if (t.type == 23) break; 
            if (t.type == T_ID && is_valid(t)) {
                char func_to_call[64];
                strcpy(func_to_call, t.value);
                fprintf(stderr, "    [Runtime] Executing Function: %s();\n", func_to_call);
                long current_pos = ftell(file);
                rewind(file);
                Token find_f;
                while ((find_f = get_next_token(file)).type != T_EOF) {
                    if (is_valid(find_f) && strcmp(find_f.value, "fun") == 0) {
                        Token name = get_next_token(file);
                        if (is_valid(name) && strcmp(name.value, func_to_call) == 0) {
                            while ((find_f = get_next_token(file)).type != 22); 
                            while ((find_f = get_next_token(file)).type != 23 && find_f.type != T_EOF) {
                                if (is_valid(find_f)) parse_statement(file, find_f);
                            }
                            break;
                        }
                    }
                }
                fseek(file, current_pos, SEEK_SET); 
            }
        }
    }
}
