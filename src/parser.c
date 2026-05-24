#include "lexer.h"
#include "ast.h" // 🌟 AST மரத்தின் ஸ்ட்ரக்சரை பார்சருக்குள் கொண்டு வருகிறோம்
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void tamizhi_gen_math_op(char* res_name, char* var1, char* op, char* var2);
void tamizhi_gen_ternary(char* res_name, char* v1, char* op, char* var2, char* true_val, char* false_val);

// 🌟 லெக்சரிலிருந்து உலகளாவிய தற்போதைய வரியை டிராக்கிங் செய்ய டிக்ளர் செய்கிறோம்
extern int current_line;

int main_generated = 0;

int is_valid(Token t) {
    if (strlen(t.value) == 0) return 0;
    return 1;
}

// 🌟 டோக்கன் வேல்யூவில் இருக்கும் தேவையற்ற ஸ்பேஸ்களை சுத்தமாக்கும் ட்ரிம் பங்க்ஷன்
void tamizhi_trim_token(char *str) {
    int len = strlen(str);
    while (len > 0 && (isspace((unsigned char)str[len - 1]) || str[len - 1] == ';')) {
        str[len - 1] = '\0';
        len--;
    }
}

void skip_to_semicolon(FILE *file) {
    Token t;
    while ((t = get_next_token(file)).type != 21 && t.type != T_EOF);
}

void parse_statement(FILE *file, Token t);

// =========================================================================
// 🌟 AST PARSER BRAIN (Recursive Descent Parsing) 🌟
// =========================================================================

// 🌟 1. எண்கள் மற்றும் மாறிகளைப் படிக்கும் அடிப்படை ஃபங்க்ஷன் (Leaf Nodes)
ASTNode* parse_primary(FILE* file, Token* current_tok) {
    if (current_tok->type == T_NUM) {
        ASTNode* node = create_number_node(atoi(current_tok->value));
        *current_tok = get_next_token(file); // அடுத்த டோக்கனுக்கு நகர்கிறோம்
        return node;
    } 
    else if (current_tok->type == T_ID) {
        ASTNode* node = create_identifier_node(current_tok->value);
        *current_tok = get_next_token(file); // அடுத்த டோக்கனுக்கு நகர்கிறோம்
        return node;
    }
    
    // தவறு நடந்தால் NULL அனுப்புகிறோம்
    return NULL;
}

// 🌟 2. கணித செயல்பாடுகளைப் படிக்கும் ஃபங்க்ஷன் (Branch Nodes)
ASTNode* parse_expression(FILE* file, Token* current_tok) {
    // முதலில் இடது பக்கம் உள்ள எண்ணையோ அல்லது மாறியையோ படிக்கிறோம் (Left node)
    ASTNode* left = parse_primary(file, current_tok);
    if (left == NULL) return NULL;

    // அடுத்து + அல்லது - அல்லது * அல்லது / வருகிறதா என்று பார்க்கிறோம்
    while (current_tok->type == 19 || current_tok->type == 56 || 
           strcmp(current_tok->value, "+") == 0 || strcmp(current_tok->value, "-") == 0 ||
           strcmp(current_tok->value, "*") == 0 || strcmp(current_tok->value, "/") == 0) {
        
        char op[10];
        strcpy(op, current_tok->value);
        *current_tok = get_next_token(file); // ஆப்பரேட்டரைத் தாண்டிச் செல்கிறோம்
        
        // வலது பக்கம் உள்ள எண்ணைப் படிக்கிறோம் (Right node)
        ASTNode* right = parse_primary(file, current_tok);
        if (right == NULL) return NULL;

        // இடது மற்றும் வலது கிளைகளை இணைத்து ஒரு புதிய மரக்கிளையை உருவாக்குகிறோம்
        left = create_binop_node(op, left, right);
    }
    
    // முழுமையாக உருவாக்கப்பட்ட மரத்தை திருப்பி அனுப்புகிறோம்
    return left;
}
// =========================================================================

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

void parse(FILE *file) {
    Token t;
    long main_pos = -1L;
    long footer_pos = -1L;
    fprintf(stderr, "\n[Parser] --- Tamizhi Engine: Universal Analysis Started ---\n");

    scan_headers(file);

    // 🌟 வரி எண் கவுண்ட்டை ரீசெட் செய்கிறோம்
    current_line = 1;

    rewind(file);
    while ((t = get_next_token(file)).type != T_EOF) {
        if (strcmp(t.value, "முதன்மை") == 0 || strcmp(t.value, "main") == 0 || strncmp(t.value, "main", 4) == 0) {
            long check_pos = ftell(file);
            Token brace_t = get_next_token(file);
            if (brace_t.type == 22 || strcmp(brace_t.value, "{") == 0) {
                main_pos = ftell(file);
            } else {
                while ((brace_t = get_next_token(file)).type != T_EOF) {
                    if (brace_t.type == 22 || strcmp(brace_t.value, "{") == 0) {
                        main_pos = ftell(file);
                        break;
                    }
                    if (strcmp(brace_t.value, "footer") == 0 || strcmp(brace_t.value, "பூட்டர்") == 0) {
                        break;
                    }
                }
            }
            if (main_pos != -1L) {
                fseek(file, check_pos, SEEK_SET);
            }
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

        while (main_brace_count > 0) {
            t = get_next_token(file);
            if (t.type == T_EOF) break;

            if (t.type == 22 || strcmp(t.value, "{") == 0) {
                main_brace_count++;
                continue;
            }
            if (t.type == 23 || strcmp(t.value, "}") == 0) {
                main_brace_count--;
                if (main_brace_count <= 0) break; 
                continue;
            }
            parse_statement(file, t);
        }
    }

    if (footer_pos != -1L) {
        fprintf(stderr, " -> Phase 3 [Footer]: Launching execution...\n");
        clearerr(file);
        fseek(file, footer_pos, SEEK_SET);

        int footer_brace_count = 1;
        while (footer_brace_count > 0) {
            t = get_next_token(file);
            if (t.type == T_EOF) break;

            if (t.type == 22 || strcmp(t.value, "{") == 0) {
                footer_brace_count++;
                continue;
            }
            if (t.type == 23 || strcmp(t.value, "}") == 0) {
                footer_brace_count--;
                if (footer_brace_count <= 0) break;
                continue;
            }

            parse_statement(file, t);

            clearerr(file);
            fseek(file, ftell(file), SEEK_SET);
        }
    }

    fprintf(stderr, "[Parser] --- Analysis Completed Successfully ---\n\n");
}

void parse_statement(FILE *file, Token t) {
    extern int var_count;
    if (!is_valid(t)) return;

    if (t.type == 21 || strcmp(t.value, ";") == 0) return;

    // 🌟 எண் (Num) மாறிகள் பிரகடனம் செய்தல்
    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {
        Token name_token = get_next_token(file); 
        Token next = get_next_token(file);

        while (next.type != 20 && next.type != T_EOF) {
            next = get_next_token(file); 
        }

        Token val_token = get_next_token(file);
        tamizhi_trim_token(val_token.value);

        if (isdigit(val_token.value[0]) || val_token.value[0] == '-') {
            tamizhi_gen_var(name_token.value, atoi(val_token.value));
        } else {
            fprintf(stderr, "[Syntax Error] வரி %d: மாறியின் மதிப்பு தவறாக உள்ளது '%s'\n", t.line, val_token.value);
        }
    }
    // 🧵 வரி (Str) மாறிகள் பிரகடனம் செய்தல்
    else if (t.type == T_STR || strcmp(t.value, "Str") == 0 || strcmp(t.value, "வரி") == 0 || strcmp(t.value, "Str ") == 0) {
        Token name_token = get_next_token(file);
        Token next = get_next_token(file);

        while (next.type != 20 && next.type != T_EOF) {
            next = get_next_token(file);
        }

        Token val_token = get_next_token(file);
        if (is_valid(val_token)) {
            tamizhi_gen_str(name_token.value, val_token.value);
        } else {
            fprintf(stderr, "[Syntax Error] வரி %d: சரத்தின் மதிப்பு விடுபட்டுள்ளது\n", t.line);
        }
    }
    // 🌟 எக்ஸ்பிரஷன்கள் மற்றும் கண்டிஷனல் ஆபரேஷன்கள் (Identifiers / Expressions)
    else if (t.type == T_ID) {
        char var_name[100];
        strcpy(var_name, t.value); 
        long current_pos = ftell(file);
        Token next_t = get_next_token(file);

        if (next_t.type == 20 || strcmp(next_t.value, "=") == 0) {
            Token v1 = get_next_token(file); 

            // 🌟 மாஸ்டர் லுக்அஹெட்: வரியின் அடுத்த 4 டோக்கன்களுக்குள் 'if' இருக்கிறதா என்று முன்கூட்டியே ஸ்கேன் செய்கிறோம்
            long lookahead_pos = ftell(file);
            int is_ternary_line = 0;
            Token check_t;

            // ஒருவேளை v1 மதிப்பே 'if' ஆக இருந்தாலும் அது டெர்னரிதான்
            if (strcmp(v1.value, "if") == 0 || v1.type == T_IF) {
                is_ternary_line = 1;
            } else {
                for (int scan_idx = 0; scan_idx < 4; scan_idx++) {
                    check_t = get_next_token(file);
                    if (strcmp(check_t.value, "if") == 0 || check_t.type == T_IF) {
                        is_ternary_line = 1;
                        break;
                    }
                    if (check_t.type == 21 || strcmp(check_t.value, ";") == 0 || check_t.type == T_EOF) {
                        break;
                    }
                }
            }

            // லுக்அஹெட் ஸ்கேன் முடிந்ததும் ஃபைல் பாயிண்டரை பழைய இடத்திற்கே ரீசெட் செய்கிறோம்
            fseek(file, lookahead_pos, SEEK_SET);

            // 🌟 1. அட்வான்ஸ்டு டெர்னரி கண்டிஷனல் எக்ஸ்பிரஷன் பைப்லைன்
            if (is_ternary_line) {
                Token op_or_keyword = get_next_token(file);

                // ஒருவேளை ஆப்செட் காரணமாக டோக்கன் மாறினால் 'if' கீவேர்டை சீரமைக்கிறோம்
                if (strcmp(op_or_keyword.value, "if") != 0 && op_or_keyword.type != T_IF) {
                    while ((op_or_keyword = get_next_token(file)).type != T_EOF) {
                        if (strcmp(op_or_keyword.value, "if") == 0 || op_or_keyword.type == T_IF) break;
                    }
                }

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

                if (next_k.type == T_ELSE || strcmp(next_k.value, "else") == 0 || strcmp(next_k.value, "இல்லையெனில்") == 0 || strcmp(next_k.value, "இல்லை எனில்") == 0) {
                    Token false_val = get_next_token(file);
                    tamizhi_gen_ternary(var_name, cond_v1.value, cond_pred.value, clean_v2, v1.value, false_val.value);
                } else {
                    fprintf(stderr, "[Syntax Error] வரி %d: டெர்னரி நிபந்தனையில் 'else' பிளாக் விடுபட்டுள்ளது\n", t.line);
                }
            }
            // 🌟 2. எல்எல்விஎம் நேடிவ் மேத்ஸ் லாஜிக் ரன்டைம்
            else {
                Token op_or_keyword = get_next_token(file);
                if (op_or_keyword.type == 19 || strcmp(op_or_keyword.value, "+") == 0 || strcmp(op_or_keyword.value, "-") == 0 || strcmp(op_or_keyword.value, "*") == 0 || strcmp(op_or_keyword.value, "/") == 0) {
                    Token v2 = get_next_token(file);
                    tamizhi_trim_token(v1.value);
                    tamizhi_trim_token(v2.value);
                    tamizhi_gen_math_op(var_name, v1.value, op_or_keyword.value, v2.value);
                }
            }
        } 
        // 🌟 நிகழ்வு (Function Call) பகுப்பாய்வு
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
                    if (is_valid(name)) {
                        char clean_fn[100];
                        strcpy(clean_fn, name.value);
                        char *b_ptr = strchr(clean_fn, '(');
                        if (b_ptr != NULL) *b_ptr = '\0';

                        if (strcmp(clean_fn, var_name) == 0) {
                            while ((find_f = get_next_token(file)).type != 22 && find_f.type != T_EOF);
                            func_body_pos = ftell(file); 
                            break;
                        }
                    }
                }
            }

            if (func_body_pos != -1L) {
                clearerr(file);
                fseek(file, func_body_pos, SEEK_SET);
                int body_brace_count = 1;
                Token body_t;

                int previous_var_count = var_count;
                
                // 🌟 மாஸ்டர் ஃபிக்ஸ்: ஃபங்க்ஷன் பாடிக்குள் நுழையும்போது மெயின் லைன் நம்பரை பேக்கப் எடுக்கிறோம்
                int backup_main_line = current_line;

                while (body_brace_count > 0 && (body_t = get_next_token(file)).type != T_EOF) {
                    if (body_t.type == 22 || strcmp(body_t.value, "{") == 0) {
                        body_brace_count++;
                    }
                    if (body_t.type == 23 || strcmp(body_t.value, "}") == 0) {
                        body_brace_count--;
                        if (body_brace_count <= 0) break;
                    }

                    if (body_t.type != 21 && strcmp(body_t.value, ";") != 0 && body_t.type != 22 && body_t.type != 23) {
                        body_t.line = current_line;
                        parse_statement(file, body_t);
                        func_body_pos = ftell(file);
                        fseek(file, func_body_pos, SEEK_SET);
                    }
                }
                var_count = previous_var_count;
                
                // ஃபங்க்ஷன் கால் முடிந்து திரும்பும்போது பழைய மெயின் லைன் கவுண்ட்டை ரீசெட் செய்கிறோம்
                current_line = backup_main_line;
            } else {
                fprintf(stderr, "[Linker Error] வரி %d: அழைக்கப்படும் நிகழ்வு வரையறுக்கப்படவில்லை '%s'\n", t.line, var_name);
            }

            clearerr(file);
            fseek(file, post_call_pos, SEEK_SET); 
        } else {
            fseek(file, current_pos, SEEK_SET);
        }
    }
    // 🌟 வெளியீடு (Print Block) பகுப்பாய்வு
    else if (t.type == T_PRINT || strcmp(t.value, "அச்சிடு") == 0) {
        Token first = get_next_token(file);
        if (first.type == 15) first = get_next_token(file); 

        if (strcmp(first.value, "Num") == 0 || strcmp(first.value, "எண்") == 0 || strcmp(first.value, "Str") == 0 || strcmp(first.value, "வரி") == 0) {
            first = get_next_token(file);
        }

        char clean_target[256];
        memset(clean_target, 0, sizeof(clean_target));
        strncpy(clean_target, first.value, sizeof(clean_target) - 1);
        tamizhi_trim_token(clean_target);

        tamizhi_gen_print(clean_target);

        long check_pos = ftell(file);
        Token semi = get_next_token(file);
        if (semi.type != 21 && strcmp(semi.value, ";") != 0) {
            fprintf(stderr, "[Warning] வரி %d: ஸ்டேட்மெண்ட்டின் இறுதியில் செமைகோலன் ';' விடுபட்டுள்ளது\n", t.line);
            fseek(file, check_pos, SEEK_SET);
        }
    }
    // 🔁 வளையம் (Loop Block) பகுப்பாய்வு
    else if (t.type == T_FOR || strcmp(t.value, "சு") == 0) {
        Token limit_token = get_next_token(file); 
        int limit = 0;

        if (isdigit(limit_token.value[0])) {
            limit = atoi(limit_token.value);
        } else {
            limit = 3; 
        }

        Token next = get_next_token(file);
        if (next.type == 22 || strcmp(next.value, "{") == 0) {
            tamizhi_gen_loop_start(limit);
            Token body_t;
            while ((body_t = get_next_token(file)).type != 23 && body_t.type != T_EOF) {
                parse_statement(file, body_t);
            }
            tamizhi_gen_loop_end();
        } else {
            fprintf(stderr, "[Syntax Error] வரி %d: சுழற்சிக்குரிய தொடக்க அடைப்புக்குறி '{' விடுபட்டுள்ளது\n", t.line);
        }
    }
}
