#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// பங்க்ஷன் அறிவிப்புகள்
void parse_statement(FILE *file, Token t);
void scan_headers(FILE *file);
void execute_footer(FILE *file, long start_pos);

// 1. மெயின் பார்ஸர்
void parse(FILE *file) {
    Token t;
    long start_pos = ftell(file);

    fprintf(stderr, "\n[Parser] --- Tamizhi Engine: Advanced 3-Pass Analysis Started ---\n");

    // PASS 1: Header Scanning
    fprintf(stderr, " -> Phase 1 [Header]: Scanning blueprints...\n");
    scan_headers(file);

    // PASS 2: Body Mapping (With Safety Checks)
    fseek(file, start_pos, SEEK_SET);
    fprintf(stderr, " -> Phase 2 [Body]: Mapping logic to DNA-VM...\n");
    while ((t = get_next_token(file)).type != T_EOF) {
        // பாதுகாப்பு: டோக்கன் வேல்யூ இல்லையென்றால் தவிர்க்கவும் (Segfault Protection)
        if (t.value == NULL || strlen(t.value) == 0) continue; 

        if (strcmp(t.value, "footer") == 0) break;

        if (strcmp(t.value, "fun") == 0) {
            Token name = get_next_token(file);
            if (name.type == T_EOF || name.value == NULL) break; 
            
            // '{' வரும் வரை தாண்டு
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); 
            
            // '}' வரும் வரை லாஜிக்கை பார்ஸ் பண்ணு
            while ((t = get_next_token(file)).type != 23 && t.type != T_EOF) {
                parse_statement(file, t);
            }
            continue;
        }
    }

    // PASS 3: Footer Execution
    fseek(file, start_pos, SEEK_SET);
    fprintf(stderr, " -> Phase 3 [Footer]: Launching execution...\n");
    execute_footer(file, start_pos);

    fprintf(stderr, "[Parser] --- Analysis Completed Successfully ---\n\n");
}

// 2. ஸ்டேட்மென்ட் இன்ஜின் (Variables, Print, If, For)
void parse_statement(FILE *file, Token t) {
    if (t.value == NULL) return;

    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {
        Token name_token = get_next_token(file); 
        while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); 
        Token first_val = get_next_token(file); 
        Token next_t = get_next_token(file);

        if (next_t.type == 19 || (next_t.value && strcmp(next_t.value, "+") == 0)) {
            Token second_val = get_next_token(file);
            tamizhi_gen_var_add(name_token.value, first_val.value, second_val.value);
        } else {
            if (first_val.value && isdigit(first_val.value[0])) {
                tamizhi_gen_var(name_token.value, atoi(first_val.value));
            }
        }
    }
    else if (t.type == T_PRINT || strcmp(t.value, "print") == 0 || strcmp(t.value, "அச்சிடு") == 0) {
        Token var_t = get_next_token(file);
        if (var_t.type == 15) var_t = get_next_token(file); 
        if (var_t.value) tamizhi_gen_print(var_t.value);
    }
    // ... (IF and FOR logic with NULL checks)
}

// 3. Phase 1: Header Scanner
void scan_headers(FILE *file) {
    Token t;
    while ((t = get_next_token(file)).type != T_EOF) {
        if (t.value && strcmp(t.value, "fun") == 0) {
            Token name = get_next_token(file);
            if (name.value) fprintf(stderr, "    [Header] Registered function: %s\n", name.value);
        }
        if (t.value && strcmp(t.value, "footer") == 0) break; 
    }
}

// 4. Phase 3: Footer Runtime Executor
void execute_footer(FILE *file, long start_pos) {
    Token t;
    int in_footer = 0;
    while ((t = get_next_token(file)).type != T_EOF) {
        if (t.value && strcmp(t.value, "footer") == 0) {
            in_footer = 1;
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); 
            continue;
        }
        if (in_footer) {
            if (t.type == 23) break; 
            if (t.type == T_ID && t.value != NULL) {
                char func_to_call[64];
                strcpy(func_to_call, t.value);
                fprintf(stderr, "    [Runtime] Booting Function: %s();\n", func_to_call);

                long current_pos = ftell(file);
                fseek(file, start_pos, SEEK_SET);

                Token find_f;
                while ((find_f = get_next_token(file)).type != T_EOF) {
                    if (find_f.value && strcmp(find_f.value, "fun") == 0) {
                        Token name = get_next_token(file);
                        if (name.value && strcmp(name.value, func_to_call) == 0) {
                            while ((find_f = get_next_token(file)).type != 22); 
                            while ((find_f = get_next_token(file)).type != 23) {
                                parse_statement(file, find_f);
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
