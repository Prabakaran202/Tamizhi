#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main_generated = 0;

int is_valid(Token t) {
    if (t.value == NULL || strlen(t.value) == 0) return 0;
    return 1;
}

void parse_statement(FILE *file, Token t);
void scan_headers(FILE *file);
void execute_footer(FILE *file, long start_pos);

void parse(FILE *file) {
    Token t;
    fprintf(stderr, "\n[Parser] --- Tamizhi Engine: Universal Analysis Started ---\n");
    scan_headers(file);
    rewind(file); 
    fprintf(stderr, " -> Phase 2 [Body]: Mapping logic to DNA-VM...\n");

    if (!main_generated) {
        tamizhi_generate_entry(); 
        main_generated = 1;
    }

    while ((t = get_next_token(file)).type != T_EOF) {
        if (!is_valid(t)) continue; 
        if (strcmp(t.value, "footer") == 0) break;
        if (strcmp(t.value, "fun") == 0 || t.type == T_FUNC) {
            int brace_count = 0;
            while ((t = get_next_token(file)).type != T_EOF) {
                if (t.type == 22) brace_count++;
                if (t.type == 23) {
                    brace_count--;
                    if (brace_count <= 0) break;
                }
            }
            continue;
        }
        parse_statement(file, t);
    }
    rewind(file);
    fprintf(stderr, " -> Phase 3 [Footer]: Launching execution...\n");
    execute_footer(file, 0L);
    fprintf(stderr, "[Parser] --- Analysis Completed Successfully ---\n\n");
}

void parse_statement(FILE *file, Token t) {
    if (!is_valid(t)) return;

    // 1. புதிய வேரியபிள் அறிவிப்பு (Num a = 0)
    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {
        Token name_token = get_next_token(file); 
        while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); 
        Token val_token = get_next_token(file);
        
        if (isdigit(val_token.value[0])) {
            tamizhi_gen_var(name_token.value, atoi(val_token.value));
        }
    }
    // 2. ⭐ புதிய அப்டேட்: ஏற்கனவே உள்ள வேரியபிளை அப்டேட் செய்தல் (a = a + 1)
    else if (t.type == T_ID) {
        char var_name[50];
        strcpy(var_name, t.value); 
        long pos_after_id = ftell(file);
        Token next = get_next_token(file);
        
        if (next.type == 20) { // '=' பார்த்தால்
            Token first_val = get_next_token(file); 
            Token op = get_next_token(file);
            if (op.type == 19 || strcmp(op.value, "+") == 0) {
                Token second_val = get_next_token(file); 
                tamizhi_gen_var_add(var_name, first_val.value, second_val.value);
            }
        } else {
            fseek(file, pos_after_id, SEEK_SET); 
        }
    }
    // 3. லூப் (for / சு)
    else if (t.type == T_FOR || strcmp(t.value, "சு") == 0 || strcmp(t.value, "for") == 0) {
        Token limit_token = get_next_token(file); 
        int limit = atoi(limit_token.value);
        Token next = get_next_token(file); 
        if (next.type == 22) { // '{'
            tamizhi_gen_loop_start(limit);
            Token body_t;
            while ((body_t = get_next_token(file)).type != 23 && body_t.type != T_EOF) {
                parse_statement(file, body_t);
            }
            tamizhi_gen_loop_end();
        }
    }
    // 4. அச்சிடு
    else if (t.type == T_PRINT || strcmp(t.value, "print") == 0 || strcmp(t.value, "அச்சிடு") == 0) {
        Token first = get_next_token(file);
        if (first.type == 15) first = get_next_token(file); // '(' ஸ்கிப்
        if (is_valid(first)) tamizhi_gen_print(first.value);
    }
}

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
