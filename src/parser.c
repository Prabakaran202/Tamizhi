#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// பாதுகாப்பிற்காக டோக்கன் செக்
int is_valid(Token t) {
    if (t.value == NULL || strlen(t.value) == 0) return 0;
    return 1;
}

void parse_statement(FILE *file, Token t);
void scan_headers(FILE *file);
void execute_footer(FILE *file, long start_pos);

void parse(FILE *file) {
    Token t;
    long start_pos = ftell(file);

    fprintf(stderr, "\n[Parser] --- Tamizhi Engine: Advanced 3-Pass Analysis Started ---\n");

    // PASS 1: ஹேடர் ஸ்கேனிங்
    scan_headers(file);

    // ---------------------------------------------------------
    // PASS 2: பாடி மேப்பிங் (யுனிவர்சல் மெயின் உருவாக்கம்)
    // ---------------------------------------------------------
    rewind(file); 
    fprintf(stderr, " -> Phase 2 [Body]: Mapping logic to DNA-VM...\n");

    // மாற்றம்: மெயின் பங்க்ஷனை இங்க ஒருமுறை மட்டும் தான் தொடங்கணும்.
    tamizhi_generate_entry(); 

    while ((t = get_next_token(file)).type != T_EOF) {
        if (!is_valid(t)) continue; 
        if (strcmp(t.value, "footer") == 0) break;

        // மாற்றம்: 'fun' உள்ளே இருக்கும் கோடு மெயினுக்குள்ள வராம இருக்க இத ஸ்கிப் பண்றோம்.
        if (strcmp(t.value, "fun") == 0) {
            // '}' வரும் வரை தாண்டு
            while ((t = get_next_token(file)).type != 23 && t.type != T_EOF); 
            continue;
        }
        
        // ஸ்டேட்மென்ட்களை அந்த ஒரே மெயின் பங்க்ஷனுக்குள்ள தள்ளுறோம்.
        parse_statement(file, t);
    }

    // ---------------------------------------------------------
    // PASS 3: பூட்டர் எக்ஸிகியூஷன்
    // ---------------------------------------------------------
    rewind(file);
    fprintf(stderr, " -> Phase 3 [Footer]: Launching execution...\n");
    
    // மாற்றம்: பூட்டர் ரன் ஆகும்போது புதுசா 'main' உருவாகாது.
    execute_footer(file, 0L);
    
    fprintf(stderr, "[Parser] --- Analysis Completed Successfully ---\n\n");
}

// 2. ஸ்டேட்மென்ட் இன்ஜின் (இதில மாற்றமில்லை, பழையபடி ஒர்க் ஆகும்)
void parse_statement(FILE *file, Token t) {
    if (!is_valid(t)) return;

    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0 || strcmp(t.value, "int") == 0) {
        Token name_token = get_next_token(file); 
        if (!is_valid(name_token)) return;
        while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); 
        Token first_val = get_next_token(file); 
        Token next_t = get_next_token(file);
        if (is_valid(next_t) && (next_t.type == 19 || strcmp(next_t.value, "+") == 0)) {
            Token second_val = get_next_token(file);
            tamizhi_gen_var_add(name_token.value, first_val.value, second_val.value);
        } else {
            if (isdigit(first_val.value[0])) tamizhi_gen_var(name_token.value, atoi(first_val.value));
        }
    }
    else if (t.type == T_PRINT || strcmp(t.value, "print") == 0 || strcmp(t.value, "அச்சிடு") == 0) {
        Token first = get_next_token(file);
        if (first.type == 15) first = get_next_token(file);
        long pos = ftell(file);
        Token op = get_next_token(file);
        if (is_valid(op) && (strcmp(op.value, "+") == 0)) {
            Token second = get_next_token(file);
            tamizhi_gen_var_add("temp_res", first.value, second.value);
            tamizhi_gen_print("temp_res");
        } 
        else {
            fseek(file, pos, SEEK_SET); 
            if (is_valid(first)) tamizhi_gen_print(first.value);
        }
    }
}

// 3. Header Scanner & Footer Logic (இதில மாற்றமில்லை)
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
