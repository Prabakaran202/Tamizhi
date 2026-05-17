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

// 🌟 புதிய பக்கா டோக்கன்-சேஃப் பிரீ-ஸ்கேன் பங்க்ஷன் (Space Skip & Body Bypass Fix)
void scan_headers(FILE *file) {
    Token t;
    rewind(file);
    fprintf(stderr, " -> Starting Header Pre-Scan...\n");

    while ((t = get_next_token(file)).type != T_EOF) {
        // 'fun' அல்லது 'நிகழ்' கீவேர்டை மிகத் துல்லியமாகப் பார்க்கிறது
        if (strcmp(t.value, "fun") == 0 || t.type == T_FUNC || strcmp(t.value, "நிகழ்") == 0) {
            Token name = get_next_token(file);

            if (is_valid(name)) {
                // 'next()' என வந்தால் பிராக்கெட்டைத் துண்டித்து பெயரை மட்டும் எடுக்க
                char *bracket_ptr = strchr(name.value, '(');
                if (bracket_ptr != NULL) {
                    *bracket_ptr = '\0';
                }

                // 'main' அல்லது 'முதன்மை' இல்லையென்றால் ரிஜிஸ்டர் செய்கிறது
                if (strcmp(name.value, "main") != 0 && strcmp(name.value, "முதன்மை") != 0) {
                    fprintf(stderr, "    [Header] Registered: %s\n", name.value);
                }
                
                // 🌟 இந்த ஃபங்ஷனோட மொத்த பாடி பிளாக்கையும் ஸ்கிப் பண்ணிக் கடக்கிறோம்
                Token skip_t;
                while ((skip_t = get_next_token(file)).type != T_EOF) {
                    if (skip_t.type == 22 || strcmp(skip_t.value, "{") == 0) {
                        int scan_brace_count = 1;
                        while ((skip_t = get_next_token(file)).type != T_EOF) {
                            if (skip_t.type == 22 || strcmp(skip_t.value, "{") == 0) scan_brace_count++;
                            if (skip_t.type == 23 || strcmp(skip_t.value, "}") == 0) {
                                scan_brace_count--;
                                if (scan_brace_count <= 0) break; // ஃபங்ஷன் பாடி முடிந்தது
                            }
                        }
                        break;
                    }
                    // ஓபனிங் பிராக்கெட் வருவதற்குள் அடுத்த 'fun' வந்தால் லூப் உடையாமல் இருக்க ரீசெட்
                    if (strcmp(skip_t.value, "fun") == 0 || strcmp(skip_t.value, "நிகழ்") == 0) {
                        long back_pos = ftell(file);
                        fseek(file, back_pos - (long)strlen(skip_t.value), SEEK_SET);
                        break;
                    }
                }
            }
        }
    }
    rewind(file); // 🌟 மிக முக்கியம்: மெயின் பாடி படிப்பதற்காக கோப்பு மீண்டும் ரீவைண்ட் செய்யப்படுகிறது!
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

    // பிராக்கெட் கவுண்ட் மூலம் மெயின் பிளாக்கை மட்டும் துல்லியமாகப் படிக்கும் லாஜிக்
    while ((t = get_next_token(file)).type != T_EOF) {
        if (strcmp(t.value, "முதன்மை") == 0 || strcmp(t.value, "main") == 0) {
            get_next_token(file); // '{' ஸ்கிப்

            int main_brace_count = 1; 

            while ((t = get_next_token(file)).type != T_EOF) {
                if (t.type == 22 || strcmp(t.value, "{") == 0) {
                    main_brace_count++;
                }
                if (t.type == 23 || strcmp(t.value, "}") == 0) {
                    main_brace_count--;
                    if (main_brace_count <= 0) {
                        break; 
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
            Token tmp;
            while ((tmp = get_next_token(file)).type != T_EOF) {
                if (tmp.type == 21 || strcmp(tmp.value, ";") == 0) {
                    break;
                }
            }
            long post_call_pos = ftell(file); 

            // Global scope-ல் பங்க்ஷன் பாடியைத் தேடி ரன் செய்கிறது
            rewind(file);
            Token find_f;
            while ((find_f = get_next_token(file)).type != T_EOF) {
                if (is_valid(find_f) && (strcmp(find_f.value, "fun") == 0 || find_f.type == T_FUNC || strcmp(find_f.value, "நிகழ்") == 0)) {
                    Token name = get_next_token(file);
                    if (is_valid(name) && strcmp(name.value, var_name) == 0) {
                        while ((find_f = get_next_token(file)).type != 22); // '{' தேடுகிறது

                        int body_brace_count = 1;
                        while ((find_f = get_next_token(file)).type != T_EOF) {
                            if (find_f.type == 22 || strcmp(find_f.value, "{") == 0) body_brace_count++;
                            if (find_f.type == 23 || strcmp(find_f.value, "}") == 0) {
                                body_brace_count--;
                                if (body_brace_count <= 0) break; 
                            }
                            parse_statement(file, find_f); 
                        }
                        break;
                    }
                }
            }
            fseek(file, post_call_pos, SEEK_SET); 
        } else {
            fseek(file, current_pos, SEEK_SET);
        }
    }
    // 4. லூப் (for / சு)
    else if (t.type == T_FOR || strcmp(t.value, "சு") == 0) {
        Token limit_token = get_next_token(file); 
        int limit = 0;

        if (isdigit(limit_token.value[0])) {
            limit = atoi(limit_token.value);
        } else {
            limit = 3; 
        }

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
    // 5. அச்சிடு (print ;)
    else if (t.type == T_PRINT || strcmp(t.value, "அச்சிடு") == 0) {
        Token first = get_next_token(file);
        if (first.type == 15) first = get_next_token(file); 
        tamizhi_gen_print(first.value);
    }
}
