#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// குளோபல் சுவிட்ச்: மெயின் பங்க்ஷன் மீண்டும் உருவாகாமல் தடுக்க
int main_generated = 0;

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

    fprintf(stderr, "\n[Parser] --- Tamizhi Engine: Universal Analysis Started ---\n");

    // PASS 1: ஹேடர் ஸ்கேனிங் (Function Registration)
    scan_headers(file);

    // ---------------------------------------------------------
    // PASS 2: பாடி மேப்பிங் (Main Logic Flow)
    // ---------------------------------------------------------
    rewind(file); 
    fprintf(stderr, " -> Phase 2 [Body]: Mapping logic to DNA-VM...\n");

    if (!main_generated) {
        tamizhi_generate_entry(); 
        main_generated = 1;
    }

    while ((t = get_next_token(file)).type != T_EOF) {
        if (!is_valid(t)) continue; 
        if (strcmp(t.value, "footer") == 0) break;

        // 'fun' பிளாக்கை ஸ்கிப் பண்ணுதல் (ஏற்கனவே ஹேடர்ல ரிஜிஸ்டர் ஆகியிருக்கும்)
        if (strcmp(t.value, "fun") == 0 || t.type == T_FUNC) {
            int brace_count = 0;
            while ((t = get_next_token(file)).type != T_EOF) {
                if (t.type == 22) brace_count++; // 22 is '{'
                if (t.type == 23) { // 23 is '}'
                    brace_count--;
                    if (brace_count <= 0) break;
                }
            }
            continue;
        }

        parse_statement(file, t);
    }

    // ---------------------------------------------------------
    // PASS 3: பூட்டர் எக்ஸிகியூஷன் (Function Calls)
    // ---------------------------------------------------------
    rewind(file);
    fprintf(stderr, " -> Phase 3 [Footer]: Launching execution...\n");
    execute_footer(file, 0L);

    fprintf(stderr, "[Parser] --- Analysis Completed Successfully ---\n\n");
}

// 2. ஸ்டேட்மென்ட் இன்ஜின் (இங்க தான் லூப் ஆட் பண்ணிருக்கோம்)
void parse_statement(FILE *file, Token t) {
    if (!is_valid(t)) return;

    // A. வேரியபிள் (Num / எண் / T_INT)
    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {
        Token name_token = get_next_token(file); 
        while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); // 20 is '='
        Token first_val = get_next_token(file); 
        Token next_t = get_next_token(file);
        
        if (is_valid(next_t) && (next_t.type == 19 || strcmp(next_t.value, "+") == 0)) {
            Token second_val = get_next_token(file);
            tamizhi_gen_var_add(name_token.value, first_val.value, second_val.value);
        } else {
            if (isdigit(first_val.value[0])) tamizhi_gen_var(name_token.value, atoi(first_val.value));
            else {
                // If it's assigning one variable to another
                tamizhi_gen_var_add(name_token.value, first_val.value, "0");
            }
        }
    }
    // B. வெளியீடு (print / அச்சிடு / T_PRINT)
    else if (t.type == T_PRINT || strcmp(t.value, "print") == 0 || strcmp(t.value, "அச்சிடு") == 0) {
        Token first = get_next_token(file);
        if (first.type == 15) first = get_next_token(file); // 15 is '('
        
        long pos = ftell(file);
        Token op = get_next_token(file);
        if (is_valid(op) && (strcmp(op.value, "+") == 0 || op.type == 19)) {
            Token second = get_next_token(file);
            tamizhi_gen_var_add("temp_res", first.value, second.value);
            tamizhi_gen_print("temp_res");
        } 
        else {
            fseek(file, pos, SEEK_SET); 
            if (is_valid(first)) tamizhi_gen_print(first.value);
        }
        // Skip semicolon if present
        pos = ftell(file);
        t = get_next_token(file);
        if (t.type != 17) fseek(file, pos, SEEK_SET); // 17 is ';'
    }
    // C. லூப் (சு / சுற்று / for / T_FOR)
    else if (t.type == T_FOR || strcmp(t.value, "சு") == 0 || strcmp(t.value, "சுற்று") == 0) {
        Token limit_token = get_next_token(file); 
        int limit = atoi(limit_token.value);
        
        Token next = get_next_token(file); 
        if (next.type == 22) { // '{'
            tamizhi_gen_loop_start(limit);
            
            Token body_t;
            while ((body_t = get_next_token(file)).type != 23 && body_t.type != T_EOF) { // '}'
                parse_statement(file, body_t);
            }
            tamizhi_gen_loop_end();
        }
    }
}

// 3. ஹேடர் ஸ்கேனிங்
void scan_headers(FILE *file) {
    Token t;
    while ((t = get_next_token(file)).type != T_EOF) {
        if (is_valid(t) && (strcmp(t.value, "fun") == 0 || t.type == T_FUNC)) {
            Token name = get_next_token(file);
            if (is_valid(name)) fprintf(stderr, "    [Header] Registered: %s\n", name.value);
        }
        if (is_valid(t) && strcmp(t.value, "footer") == 0) break; 
    }
}

// 4. பூட்டர் எக்ஸிகியூஷன்
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
                long current_pos = ftell(file);
                rewind(file);
                Token find_f;
                while ((find_f = get_next_token(file)).type != T_EOF) {
                    if (is_valid(find_f) && (strcmp(find_f.value, "fun") == 0 || find_f.type == T_FUNC)) {
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
