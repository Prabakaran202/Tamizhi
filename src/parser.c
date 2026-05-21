#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// 🌟 புதிய எல்எல்விஎம் மேத்ஸ் மற்றும் டெர்னரி ஆபரேட்டர் ஃபங்ஷன் டிக்ளரேஷன்கள்
void tamizhi_gen_math_op(char* res_name, char* var1, char* op, char* var2);
void tamizhi_gen_ternary(char* res_name, char* v1, char* op, char* var2, char* true_val, char* false_val);

// 🌟 எல்எல்விஎம் மெயின் பங்க்ஷன் (main) ஒருமுறைக்கு மேல் டூப்ளிகேட் ஆகாமல் தடுக்க உதவும் ஃப்ளாக்
int main_generated = 0;

// 🌟 டோக்கன் வேல்யூ காலியாக இல்லாமல் சரியாக உள்ளதா என சரிபார்க்கும் பங்க்ஷன்
int is_valid(Token t) {
    if (strlen(t.value) == 0) return 0;
    return 1;
}

// 🌟 செமிகோலன் (;) வரும் வரை இடையில் இருக்கும் தேவையற்ற டோக்கன்களை தவிர்த்து கடக்க உதவும் பங்க்ஷன்
void skip_to_semicolon(FILE *file) {
    Token t;
    while ((t = get_next_token(file)).type != 21 && t.type != T_EOF);
}

void parse_statement(FILE *file, Token t);

// 🌟 ஹெடர் பிரீ-ஸ்கேன் (Pre-Scan): கோப்பு முழுக்க பங்க்ஷன் (fun) பெயர்களை மட்டும் முன்கூட்டியே ரிஜிஸ்டர் செய்யும் இடம்
void scan_headers(FILE *file) {
    Token t;
    rewind(file);
    fprintf(stderr, " -> Starting Header Pre-Scan...\n");

    while ((t = get_next_token(file)).type != T_EOF) {
        if (strcmp(t.value, "fun") == 0 || t.type == T_FUNC || strcmp(t.value, "நிகழ்") == 0) {
            Token name = get_next_token(file);

            if (is_valid(name)) {
                char *bracket_ptr = strchr(name.value, '(');
                if (bracket_ptr != NULL) {
                    *bracket_ptr = '\0';
                }

                if (strcmp(name.value, "main") != 0 && strcmp(name.value, "முதன்மை") != 0) {
                    fprintf(stderr, "    [Header] Registered: %s\n", name.value);
                }

                Token skip_t;
                while ((skip_t = get_next_token(file)).type != T_EOF) {
                    if (skip_t.type == 22 || strcmp(skip_t.value, "{") == 0) {
                        int scan_brace_count = 1;
                        while ((skip_t = get_next_token(file)).type != T_EOF) {
                            if (skip_t.type == 22 || strcmp(skip_t.value, "{") == 0) scan_brace_count++;
                            if (skip_t.type == 23 || strcmp(skip_t.value, "}") == 0) {
                                scan_brace_count--;
                                if (skip_t.type == 23 || scan_brace_count <= 0) break;
                            }
                        }
                        break;
                    }
                    if (strcmp(skip_t.value, "fun") == 0 || strcmp(skip_t.value, "நிகழ்") == 0) {
                        long back_pos = ftell(file);
                        fseek(file, back_pos - (long)strlen(skip_t.value), SEEK_SET);
                        break;
                    }
                }
            }
        }
    }
    rewind(file);
}

// 🌟 பிரதான பார்ஸர் இன்ஜின் (Main Parser Engine): main மற்றும் footer பிளாக்குகளை இயக்கும் இடம்
void parse(FILE *file) {
    Token t;
    long main_pos = -1L;
    long footer_pos = -1L;
    fprintf(stderr, "\n[Parser] --- Tamizhi Engine: Universal Analysis Started ---\n");

    scan_headers(file);

    rewind(file);
    while ((t = get_next_token(file)).type != T_EOF) {
        if (strcmp(t.value, "முதன்மை") == 0 || strcmp(t.value, "main") == 0) {
            Token brace_t = get_next_token(file);
            if (brace_t.type != 22 && strcmp(brace_t.value, "{") != 0) {
                while ((brace_t = get_next_token(file)).type != 22 && brace_t.type != T_EOF);
            }
            main_pos = ftell(file);
        }
        else if (strcmp(t.value, "பூட்டர்") == 0 || strcmp(t.value, "footer") == 0) {
            Token brace_t = get_next_token(file);
            if (brace_t.type != 22 && strcmp(brace_t.value, "{") != 0) {
                while ((brace_t = get_next_token(file)).type != 22 && brace_t.type != T_EOF);
            }
            footer_pos = ftell(file);
        }
    }

    fprintf(stderr, " -> Phase 2 [Body]: Mapping logic to DNA-VM...\n");
    if (!main_generated) {
        tamizhi_generate_entry(); 
        main_generated = 1;
    }

    if (main_pos != -1L) {
        clearerr(file);
        fseek(file, main_pos, SEEK_SET);
        int main_brace_count = 1;

        while (main_brace_count > 0 && (t = get_next_token(file)).type != T_EOF) {
            if (t.type == 22 || strcmp(t.value, "{") == 0) {
                main_brace_count++;
            }
            if (t.type == 23 || strcmp(t.value, "}") == 0) {
                main_brace_count--;
                if (main_brace_count <= 0) break; 
            }
            parse_statement(file, t);
        }
    }

    if (footer_pos != -1L) {
        fprintf(stderr, " -> Phase 3 [Footer]: Launching execution...\n");
        clearerr(file);
        fseek(file, footer_pos, SEEK_SET);

        int footer_brace_count = 1;
        while (footer_brace_count > 0 && (t = get_next_token(file)).type != T_EOF) {
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

// 🌟 ஸ்டேட்மென்ட் அனலைசர் (Statement Analyzer): வரிகளை பகுப்பாய்வு செய்யும் பிரதான இடம்
void parse_statement(FILE *file, Token t) {
    extern int var_count;
    if (!is_valid(t)) return;

    if (t.type == 21 || strcmp(t.value, ";") == 0) return;

    // 1️⃣ எண்கள் பிளாக் (Num a = 10 ;) -> முழு எண்களை டிக்ளேர் செய்ய
    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {
        Token name_token = get_next_token(file); 
        Token next = get_next_token(file);
        while (next.type != 20 && next.type != T_EOF) {
            next = get_next_token(file); 
        }
        Token val_token = get_next_token(file);
        if (isdigit(val_token.value[0])) {
            tamizhi_gen_var(name_token.value, atoi(val_token.value));
        }
    }
    // 2️⃣ சரங்கள் பிளாக் (Str s = "Hello" ;) -> உரைகளை டிக்ளேர் செய்ய
    else if (t.type == T_STR || strcmp(t.value, "Str") == 0 || strcmp(t.value, "வரி") == 0) {
        Token name_token = get_next_token(file);
        Token next = get_next_token(file);
        while (next.type != 20 && next.type != T_EOF) {
            next = get_next_token(file);
        }
        Token val_token = get_next_token(file);
        if (is_valid(val_token)) {
            tamizhi_gen_str(name_token.value, val_token.value);
        }
    }
    // 3️⃣ ஐடென்டிஃபையர் பிளாக் (Identifier Block): மேத்ஸ் ஆபரேஷன், டெர்னரி எக்ஸ்பிரஷன் அல்லது ஃபங்ஷன் கால்கள்
    else if (t.type == T_ID) {
        char var_name[50];
        strcpy(var_name, t.value); 
        long current_pos = ftell(file);
        Token next_t = get_next_token(file);

        // 🅰️ '=' குறியீடு இருந்தால் உள்ளே நுழையும்
        if (next_t.type == 20 || strcmp(next_t.value, "=") == 0) {
            Token v1 = get_next_token(file);
            Token op_or_keyword = get_next_token(file);
            
            // 🌟 அட்வான்ஸ்டு டெர்னரி சிங்கிள்-லைன் கண்டிஷனல் அசைன்மென்ட் ஃபீச்சர் (Ternary Flow)
            if (strcmp(op_or_keyword.value, "if") == 0 || strcmp(op_or_keyword.value, "எனில்") == 0) {
                Token cond_v1 = get_next_token(file);
                if (strcmp(cond_v1.value, "(") == 0) {
                    cond_v1 = get_next_token(file);
                }
                Token cond_pred = get_next_token(file);
                Token cond_v2 = get_next_token(file);
                
                char clean_v2[50];
                strcpy(clean_v2, cond_v2.value);
                char *end_p = strchr(clean_v2, ')');
                if (end_p != NULL) *end_p = '\0';
                
                Token next_k = get_next_token(file);
                if (strcmp(next_k.value, ")") == 0) {
                    next_k = get_next_token(file);
                }
                
                if (strcmp(next_k.value, "else") == 0 || strcmp(next_k.value, "இல்லை எனில்") == 0) {
                    Token false_val = get_next_token(file);
                    tamizhi_gen_ternary(var_name, cond_v1.value, cond_pred.value, clean_v2, v1.value, false_val.value);
                }
            }
            // 🌟 சாதாரண கணித ஆபரேஷன்கள் பிளாக் (+, -, *, /)
            else if (op_or_keyword.type == 19 || strcmp(op_or_keyword.value, "+") == 0 || strcmp(op_or_keyword.value, "-") == 0 || strcmp(op_or_keyword.value, "*") == 0 || strcmp(op_or_keyword.value, "/") == 0) {
                Token v2 = get_next_token(file);
                tamizhi_gen_math_op(var_name, v1.value, op_or_keyword.value, v2.value);
            }
        } 
        // 🅱️ '(' இருந்தால் பூட்டர் வழியா நடக்குற ஃபங்ஷன் இன்லைன் கால்
        else if (next_t.type == 15 || strcmp(next_t.value, "(") == 0) {
            Token tmp;
            while ((tmp = get_next_token(file)).type != T_EOF) {
                if (tmp.type == 21 || strcmp(tmp.value, ";") == 0) {
                    break;
                }
            }
            long post_call_pos = ftell(file);

            clearerr(file);
            rewind(file);
            Token find_f;
            long func_body_pos = -1L;

            while ((find_f = get_next_token(file)).type != T_EOF) {
                if (is_valid(find_f) && (strcmp(find_f.value, "fun") == 0 || find_f.type == T_FUNC || strcmp(find_f.value, "நிகழ்") == 0)) {
                    Token name = get_next_token(file);
                    if (is_valid(name) && strcmp(name.value, var_name) == 0) {
                        while ((find_f = get_next_token(file)).type != 22 && find_f.type != T_EOF);
                        func_body_pos = ftell(file); 
                        break;
                    }
                }
            }

            // 🌟 லோக்கல் ஸ்கோப் மெமரி மேனேஜ்மென்ட் லாஜிக்
            if (func_body_pos != -1L) {
                clearerr(file);
                fseek(file, func_body_pos, SEEK_SET);
                int body_brace_count = 1;
                Token body_t;

                int previous_var_count = var_count;

                while (body_brace_count > 0 && (body_t = get_next_token(file)).type != T_EOF) {
                    if (body_t.type == 22 || strcmp(body_t.value, "{") == 0) {
                        body_brace_count++;
                    }
                    if (body_t.type == 23 || strcmp(body_t.value, "}") == 0) {
                        body_brace_count--;
                        if (body_brace_count <= 0) break;
                    }

                    if (body_t.type != 21 && strcmp(body_t.value, ";") != 0 && body_t.type != 22 && body_t.type != 23) {
                        parse_statement(file, body_t);
                        func_body_pos = ftell(file);
                        fseek(file, func_body_pos, SEEK_SET);
                    }
                }
                var_count = previous_var_count; // லோக்கல் பங்க்ஷன் முடிந்ததும் மெமரியை கச்சிதமாக ரீசெட் செய்கிறது
            }

            clearerr(file);
            fseek(file, post_call_pos, SEEK_SET); 
        } else {
            fseek(file, current_pos, SEEK_SET);
        }
    }
    // 4️⃣ அச்சிடு பிளாக் (print ;) -> திரையில் அவுட்புட் காட்ட
    else if (t.type == T_PRINT || strcmp(t.value, "அச்சிடு") == 0) {
        Token first = get_next_token(file);
        if (first.type == 15) first = get_next_token(file); 

        if (strcmp(first.value, "Num") == 0 || strcmp(first.value, "எண்") == 0 || strcmp(first.value, "Str") == 0 || strcmp(first.value, "வரி") == 0) {
            first = get_next_token(file);
        }

        char clean_target[256];
        memset(clean_target, 0, sizeof(clean_target));
        strncpy(clean_target, first.value, sizeof(clean_target) - 1);

        tamizhi_gen_print(clean_target);

        long check_pos = ftell(file);
        Token semi = get_next_token(file);
        if (semi.type != 21 && strcmp(semi.value, ";") != 0) {
            fseek(file, check_pos, SEEK_SET);
        }
    }
    // 5️⃣ லூப் பிளாக் (for / சு) -> லூப்களைக் கையாளுவதற்கு
    else if (t.type == T_FOR || strcmp(t.value, "சு") == 0) {
        Token limit_token = get_next_token(file); 
        int limit = 0;

        if (isdigit(limit_token.value[0])) {
            limit = atoi(limit_token.value);
        } else {
            limit = 3; 
        }

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
    // 6️⃣ மல்டி-லைன் இஃப்-எல்ஸ் பிளாக் (if / எனில்) -> நிபந்தனைகளைக் கையாள
    else if (strcmp(t.value, "if") == 0 || strcmp(t.value, "எனில்") == 0) {
        Token next_t = get_next_token(file);
        if (strcmp(next_t.value, "(") == 0) {
            next_t = get_next_token(file);
        }

        char v1_buf[50], op_buf[10], v2_buf[50];
        strcpy(v1_buf, next_t.value);

        Token op_t = get_next_token(file);
        strcpy(op_buf, op_t.value);

        Token v2_t = get_next_token(file);
        char *end_bracket_ptr = strchr(v2_t.value, ')');
        if (end_bracket_ptr != NULL) {
            *end_bracket_ptr = '\0';
        }
        strcpy(v2_buf, v2_t.value);

        Token next = get_next_token(file);
        if (strcmp(next.value, ")") == 0) {
            next = get_next_token(file);
        }

        tamizhi_gen_if_start(v1_buf, op_buf, v2_buf);

        if (next.type == 22 || strcmp(next.value, "{") == 0) {
            int if_brace_count = 1;
            Token if_body_t;
            while (if_brace_count > 0 && (if_body_t = get_next_token(file)).type != T_EOF) {
                if (if_body_t.type == 22 || strcmp(if_body_t.value, "{") == 0) if_brace_count++;
                if (if_body_t.type == 23 || strcmp(if_body_t.value, "}") == 0) {
                    if_brace_count--;
                    if (if_brace_count <= 0) break;
                }
                parse_statement(file, if_body_t);
            }
        }

        long else_pos = ftell(file);
        Token else_t = get_next_token(file);
        if (strcmp(else_t.value, "else") == 0 || strcmp(else_t.value, "இல்லை எனில்") == 0) {
            tamizhi_gen_else_start();
            Token else_brace = get_next_token(file);
            if (else_brace.type == 22 || strcmp(else_brace.value, "{") == 0) {
                int else_brace_count = 1;
                Token else_body_t;
                while (else_brace_count > 0 && (else_body_t = get_next_token(file)).type != T_EOF) {
                    if (else_body_t.type == 22 || strcmp(else_body_t.value, "{") == 0) else_brace_count++;
                    if (else_body_t.type == 23 || strcmp(else_body_t.value, "}") == 0) {
                        else_brace_count--;
                        if (else_brace_count <= 0) break;
                    }
                    parse_statement(file, else_body_t);
                }
            }
        } else {
            fseek(file, else_pos, SEEK_SET);
        }
        tamizhi_gen_if_end();
    }
}
