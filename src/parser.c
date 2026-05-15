#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main_generated = 0;

// லேப்டாப் வார்னிங்கைத் தவிர்க்க திருத்தப்பட்ட பங்க்ஷன்
int is_valid(Token t) {
    if (strlen(t.value) == 0) return 0;
    return 1;
}

// 🌟 புதிய ஹெல்பர்: செமிகோலன் (;) வரும் வரை டோக்கன்களைத் தவிர்க்க
void skip_to_semicolon(FILE *file) {
    Token t;
    // Lexer-ல் செமிகோலனுக்கு டைப் 21 கொடுத்துள்ளோம்
    while ((t = get_next_token(file)).type != 21 && t.type != T_EOF);
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
        
        // பங்க்ஷன் டெபனிஷன்களைத் தவிர்க்க
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

    // 1. எண்கள் (Num a = 10 ;)
    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {
        Token name_token = get_next_token(file); 
        while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); 
        Token val_token = get_next_token(file);

        if (isdigit(val_token.value[0])) {
            tamizhi_gen_var(name_token.value, atoi(val_token.value));
        }
        skip_to_semicolon(file); // 🌟 அடுத்த வரிக்கு நகர
    }
    // 2. சரங்கள் (Str s = "Hello" ;)
    else if (t.type == T_STR || strcmp(t.value, "Str") == 0 || strcmp(t.value, "வரி") == 0) {
        Token name_token = get_next_token(file);
        while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); 
        Token val_token = get_next_token(file);

        if (is_valid(val_token)) {
            tamizhi_gen_str(name_token.value, val_token.value);
        }
        skip_to_semicolon(file); // 🌟 அடுத்த வரிக்கு நகர
    }
    // 3. வேரியபிள் அப்டேட் (a = a + 1 ;)
    else if (t.type == T_ID) {
        char var_name[50];
        strcpy(var_name, t.value); 
        long pos_after_id = ftell(file);
        Token next = get_next_token(file);

        if (next.type == 20) { 
            Token first_val = get_next_token(file); 
            Token op = get_next_token(file);
            if (op.type == 19 || strcmp(op.value, "+") == 0) {
                Token second_val = get_next_token(file); 
                tamizhi_gen_var_add(var_name, first_val.value, second_val.value);
            }
            skip_to_semicolon(file); // 🌟 அடுத்த வரிக்கு நகர
        } else {
            fseek(file, pos_after_id, SEEK_SET); 
        }
    }
    // 4. லூப் (for / சு)
    else if (t.type == T_FOR || strcmp(t.value, "சு") == 0 || strcmp(t.value, "for") == 0) {
        Token limit_token = get_next_token(file); 
        int limit = atoi(limit_token.value);
        Token next = get_next_token(file); 
        if (next.type == 22) { 
            tamizhi_gen_loop_start(limit);
            Token body_t;
            while ((body_t = get_next_token(file)).type != 23 && body_t.type != T_EOF) {
                parse_statement(file, body_t);
            }
            tamizhi_gen_loop_end();
        }
    }
    // 5. அச்சிடு (print ;)
    else if (t.type == T_PRINT || strcmp(t.value, "print") == 0 || strcmp(t.value, "அச்சிடு") == 0) {
        Token first = get_next_token(file);
        if (first.type == 15) first = get_next_token(file); 
        if (is_valid(first)) {
            tamizhi_gen_print(first.value);
        }
        // பிராக்கெட் இருந்தால் அதை ஸ்கிப் செய்ய
        Token check = get_next_token(file);
        if (check.type != 21) skip_to_semicolon(file);
    }
}

// scan_headers மற்றும் execute_footer பழையபடி இருக்கும்
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
