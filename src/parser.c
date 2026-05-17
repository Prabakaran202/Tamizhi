#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main_generated = 0;

int is_valid(Token t) {
    if (strlen(t.value) == 0) return 0;
    return 1;
}

// 🌟 செமிகோலன் (;) வரும் வரை டோக்கன்களைத் தவிர்க்க
void skip_to_semicolon(FILE *file) {
    Token t;
    while ((t = get_next_token(file)).type != 21 && t.type != T_EOF);
}

void parse_statement(FILE *file, Token t);

// 🌟 புதிய கச்சிதமான ஹெடர் பிரீ-ஸ்கேன் பங்க்ஷன்
void scan_headers(FILE *file) {
    Token t;
    rewind(file);
    fprintf(stderr, " -> Starting Header Pre-Scan...\n");
    
    while ((t = get_next_token(file)).type != T_EOF) {
        if (is_valid(t) && (strcmp(t.value, "fun") == 0 || t.type == T_FUNC || strcmp(t.value, "நிகழ்") == 0)) {
            Token name = get_next_token(file);
            if (is_valid(name)) {
                fprintf(stderr, "    [Header] Registered: %s\n", name.value);
            }
        }
    }
    rewind(file); // 🌟 பிரீ-ஸ்கேன் முடிந்ததும் கோப்பு தொடக்கப்புள்ளிக்குத் திரும்புகிறது!
}

void parse(FILE *file) {
    Token t;
    long footer_pos = -1L;
    fprintf(stderr, "\n[Parser] --- Tamizhi Engine: Universal Analysis Started ---\n");

    // Phase 1: பங்க்ஷன்களை மட்டும் ஸ்கேன் செய்து ரிஜிஸ்டர் செய்தல்
    scan_headers(file);

    // Phase 2: பூட்டர் (Footer) எங்குள்ளது எனத் தேடுதல்
    rewind(file);
    while ((t = get_next_token(file)).type != T_EOF) {
        if (strcmp(t.value, "பூட்டர்") == 0 || strcmp(t.value, "footer") == 0) {
            footer_pos = ftell(file);
            break;
        }
    }

    // Phase 3: மெயின் (Main) பகுதிக்குள் இருப்பவற்றை இயக்குதல்
    rewind(file);
    fprintf(stderr, " -> Phase 2 [Body]: Mapping logic to DNA-VM...\n");
    if (!main_generated) {
        tamizhi_generate_entry(); 
        main_generated = 1;
    }

    // 🌟 பிராக்கெட் கவுண்ட் மூலம் மெயின் பிளாக்கை மட்டும் துல்லியமாகப் படிக்கும் லாஜிக்
    while ((t = get_next_token(file)).type != T_EOF) {
        if (strcmp(t.value, "முதன்மை") == 0 || strcmp(t.value, "main") == 0) {
            get_next_token(file); // '{' ஸ்கிப்

            int main_brace_count = 1; // மெயின் ஓபனிங் பிராக்கெட்டைக் கணக்கில் கொள்கிறோம்

            while ((t = get_next_token(file)).type != T_EOF) {
                if (t.type == 22 || strcmp(t.value, "{") == 0) {
                    main_brace_count++;
                }
                if (t.type == 23 || strcmp(t.value, "}") == 0) {
                    main_brace_count--;
                    if (main_brace_count <= 0) {
                        break; // மெயின் பிளாக்கோட உண்மையான முடிவு பிராக்கெட் வந்தால் மட்டுமே வெளியேறும்!
                    }
                }
                parse_statement(file, t);
            }
            break; 
        }
    }

    // Phase 4: பூட்டர் (Footer) பகுதிக்குத் தாவி இயக்குதல்
    if (footer_pos != -1L) {
        fprintf(stderr, " -> Phase 3 [Footer]: Launching execution...\n");
        fseek(file, footer_pos, SEEK_SET);
        get_next_token(file); // '{' ஸ்கிப்

        int footer_brace_count = 1;
        while ((t = get_next_token(file)).type != T_EOF) {
            if (t.type == 22 || strcmp(t.value, "{") == 0) footer_brace_count++;
            if (t.type == 23 || strcmp(t.value, "}") == 0) {
                footer_brace_count--;
                if (footer_brace_count <= 0) break;
            }
            parse_statement(file, t);
        }
    }

    fprintf(stderr, "[Parser] --- Analysis Completed Successfully ---\n\n");
}

void parse_statement(FILE *file, Token t) {
    if (!is_valid(t)) return;

    // செமிகோலன் (;) அல்லது நியூலைன் டோக்கனாக இருந்தால் அதை அப்படியே கடந்து செல்லலாம்
    if (t.type == 21 || strcmp(t.value, ";") == 0) return;

    // 1. எண்கள் (Num a = 10 ;)
    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {
        Token name_token = get_next_token(file); 
        while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); 
        Token val_token = get_next_token(file);
        if (isdigit(val_token.value[0])) {
            tamizhi_gen_var(name_token.value, atoi(val_token.value));
        }
    }
    // 2. சரங்கள் (Str s = "Hello" ;)
    else if (t.type == T_STR || strcmp(t.value, "Str") == 0 || strcmp(t.value, "வரி") == 0) {
        Token name_token = get_next_token(file);
        while ((t = get_next_token(file)).type != 20 && t.type != T_EOF); 
        Token val_token = get_next_token(file);
        if (is_valid(val_token)) {
            tamizhi_gen_str(name_token.value, val_token.value);
        }
    }
    // 3. வேரியபிள் அப்டேட் அல்லது பங்க்ஷன் கால் (Fix: Sequential Multi-line Execution) 🌟
    else if (t.type == T_ID) {
        char var_name[50];
        strcpy(var_name, t.value); 
        long current_pos = ftell(file);
        Token next_t = get_next_token(file);

        if (next_t.type == 20) { // '=' -> வேரியபிள் அப்டேட்
            Token v1 = get_next_token(file);
            Token op = get_next_token(file);
            if (op.type == 19) { // '+'
                Token v2 = get_next_token(file);
                tamizhi_gen_var_add(var_name, v1.value, v2.value);
            }
        } 
        else if (next_t.type == 15 || strcmp(next_t.value, "(") == 0) { // '(' -> பங்க்ஷன் கால்
            // STEP 1: Function call முடிகிற செமிகோலன் வரை மட்டும் படித்து அதன் போஸ்ட்-பொசிஷனை எடுக்கிறோம்
            Token tmp;
            while ((tmp = get_next_token(file)).type !=
