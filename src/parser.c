#include "lexer.h"
#include "ast.h"
#include "codegen.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void tamizhi_gen_math_op(char* res_name, char* var1, char* op, char* var2);
void tamizhi_gen_ternary(char* res_name, char* v1, char* op, char* var2, char* true_val, char* false_val);

extern int current_line;
extern LLVMValueRef current_function; // 🌟 codegen-ல் இருந்து தற்போதைய ஃபங்ஷன் பாயிண்டரை வாங்குகிறோம்
extern LLVMModuleRef module;          // 🌟 குளோபல் மாட்யூல் ரெஃபரன்ஸ்
extern LLVMContextRef context;         // 🌟 குளோபல் கான்டெக்ஸ்ட் ரெஃபரன்ஸ்

int main_generated = 0;

#define MAX_CALL_DEPTH 100
int call_depth = 0;

// ==========================================================
// Helpers
// ==========================================================

int is_valid(Token t) {
    return strlen(t.value) > 0;
}

void tamizhi_trim_token(char *str) {
    int len = strlen(str);

    while (
        len > 0 &&
        (
            isspace((unsigned char)str[len - 1]) ||
            str[len - 1] == ';'
        )
    ) {
        str[len - 1] = '\0';
        len--;
    }
}

// FIX: வரியின் இறுதி செமிகோலன் வரை டோக்கன்களைக் கடந்து செல்ல
void skip_to_semicolon(FILE *file) {
    Token t;
    while (
        (t = get_next_token(file)).type != 21 &&
        t.type != T_EOF
    );
}

// FIX: டோக்கன் ஏற்கனவே ';' ஐ உட்கொண்டிருந்தால் டபுள்-ஸ்கிப் ஆவதைத் தடுக்கிறது
void skip_remaining_if_needed(FILE *file, Token current_tok) {
    if (current_tok.type != 21 && strcmp(current_tok.value, ";") != 0 && current_tok.type != T_EOF) {
        skip_to_semicolon(file);
    }
}

void parse_statement(FILE *file, Token t);

// ==========================================================
// AST Memory Cleanup
// ==========================================================

void free_ast(ASTNode* node) {
    if (!node) return;

    if (node->type == AST_BINARY_OP) {
        free_ast(node->data.binop.left);
        free_ast(node->data.binop.right);
    }

    free(node);
}

// ==========================================================
// AST Parser (BODMAS Precedence + Parenthesis Map)
// ==========================================================

ASTNode* parse_expression(FILE* file, Token* current_tok);

ASTNode* parse_primary(FILE* file, Token* current_tok) {

    // Parenthesis Support - ( அடைப்புக்குறிகளுக்கு முன்னுரிமை அளிக்கிறது )
    if (strcmp(current_tok->value, "(") == 0) {

        *current_tok = get_next_token(file);

        ASTNode* expr = parse_expression(file, current_tok);

        if (strcmp(current_tok->value, ")") != 0) {
            fprintf(stderr, "[Syntax Error] Missing ')'\n");
            return NULL;
        }

        *current_tok = get_next_token(file);

        return expr;
    }

    // Number
    if (
        current_tok->type == T_NUM ||
        isdigit((unsigned char)current_tok->value[0]) ||
        (
            current_tok->value[0] == '-' &&
            isdigit((unsigned char)current_tok->value[1])
        )
    ) {

        ASTNode* node = create_number_node(atoi(current_tok->value));
        *current_tok = get_next_token(file);
        return node;
    }

    // Identifier
    if (current_tok->type == T_ID) {

        ASTNode* node = create_identifier_node(current_tok->value);
        *current_tok = get_next_token(file);
        return node;
    }

    return NULL;
}

// Higher Precedence: * மற்றும் /
ASTNode* parse_term(FILE* file, Token* current_tok) {

    ASTNode* left = parse_primary(file, current_tok);

    while (
        strcmp(current_tok->value, "*") == 0 ||
        strcmp(current_tok->value, "/") == 0
    ) {

        char op[10];
        strcpy(op, current_tok->value);

        *current_tok = get_next_token(file);

        ASTNode* right = parse_primary(file, current_tok);

        if (!right) return NULL;

        left = create_binop_node(op, left, right);
    }

    return left;
}

// Lower Precedence: + மற்றும் -
ASTNode* parse_expression(FILE* file, Token* current_tok) {

    ASTNode* left = parse_term(file, current_tok);

    while (
        strcmp(current_tok->value, "+") == 0 ||
        strcmp(current_tok->value, "-") == 0
    ) {

        char op[10];
        strcpy(op, current_tok->value);

        *current_tok = get_next_token(file);

        ASTNode* right = parse_term(file, current_tok);

        if (!right) return NULL;

        left = create_binop_node(op, left, right);
    }

    return left;
}

// ==========================================================
// Function Table
// ==========================================================

typedef struct {
    char name[100];
    long pos;
} FunctionEntry;

FunctionEntry functions[256];
int function_count = 0;

long find_function(const char* name) {

    for (int i = 0; i < function_count; i++) {

        if (strcmp(functions[i].name, name) == 0) {
            return functions[i].pos;
        }
    }

    return -1L;
}

// ==========================================================
// Header Scan
// ==========================================================

void scan_headers(FILE *file) {

    Token t;
    rewind(file);

    fprintf(stderr, " -> Starting Header Pre-Scan...\n");

    while ((t = get_next_token(file)).type != T_EOF) {

        if (
            strcmp(t.value, "fun") == 0 ||
            strcmp(t.value, "நிகழ்") == 0 ||
            t.type == T_FUNC
        ) {

            Token name = get_next_token(file);

            if (!is_valid(name))
                continue;

            char clean_name[100];
            strcpy(clean_name, name.value);

            char* bracket = strchr(clean_name, '(');
            if (bracket)
                *bracket = '\0';

            Token tk;

            while ((tk = get_next_token(file)).type != T_EOF) {

                if (tk.type == 22 || strcmp(tk.value, "{") == 0) {

                    strcpy(functions[function_count].name, clean_name);
                    functions[function_count].pos = ftell(file);
                    function_count++;

                    fprintf(stderr, "    [Header] Registered: %s\n", clean_name);
                    break;
                }
            }
        }
    }

    rewind(file);
}

// ==========================================================
// Main Parse
// ==========================================================

void parse(FILE *file) {
    Token t;
    long main_pos = -1L;
    long footer_pos = -1L;

    fprintf(stderr, "\n[Parser] --- Tamizhi Engine Started ---\n");

    scan_headers(file);

    long check_main = find_function("main");
    if (check_main != -1L) {
        main_pos = check_main;
    } else {
        main_pos = find_function("முதன்மை");
    }

    long check_footer = find_function("footer");
    if (check_footer != -1L) {
        footer_pos = check_footer;
    } else {
        footer_pos = find_function("பூட்டர்");
    }

    if (main_pos == -1L || footer_pos == -1L) {
        rewind(file);
        while ((t = get_next_token(file)).type != T_EOF) {
            if (strcmp(t.value, "main") == 0 || strcmp(t.value, "முதன்மை") == 0) {
                main_pos = ftell(file);
            }
            else if (strcmp(t.value, "footer") == 0 || strcmp(t.value, "பூட்டர்") == 0) {
                footer_pos = ftell(file);
            }
        }
    }

    tamizhi_generate_entry();

    clearerr(file); 
    rewind(file); 

    if (main_pos != -1L) {
        fseek(file, main_pos, SEEK_SET);
        Token check_brace = get_next_token(file);
        if (strcmp(check_brace.value, "{") != 0) {
            fseek(file, main_pos, SEEK_SET);
        }

        current_function = LLVMGetNamedFunction(module, "main");

        int brace_count = 1;
        while (brace_count > 0) {
            t = get_next_token(file);
            if (t.type == T_EOF) break;

            if (t.type == 22 || strcmp(t.value, "{") == 0) {
                brace_count++;
                continue;
            }

            if (t.type == 23 || strcmp(t.value, "}") == 0) {
                brace_count--;
                if (brace_count <= 0) break;
                continue;
            }

            parse_statement(file, t);
        }
    }

    if (footer_pos != -1L) {
        fseek(file, footer_pos, SEEK_SET);
        Token check_brace = get_next_token(file);
        if (strcmp(check_brace.value, "{") != 0) {
            fseek(file, footer_pos, SEEK_SET);
        }

        int brace_count = 1;
        while (brace_count > 0) {
            t = get_next_token(file);
            if (t.type == T_EOF) break;

            if (t.type == 22 || strcmp(t.value, "{") == 0) {
                brace_count++;
                continue;
            }

            if (t.type == 23 || strcmp(t.value, "}") == 0) {
                brace_count--;
                if (brace_count <= 0) break;
                continue;
            }

            parse_statement(file, t);
        }
    }

    fprintf(stderr, "[Parser] --- Completed Successfully ---\n");
}

// ==========================================================
// Statement Parser
// ==========================================================

void parse_statement(FILE *file, Token t) {

    extern int var_count;

    if (!is_valid(t)) return;
    if (t.type == 21 || strcmp(t.value, ";") == 0) return;

    if (strcmp(t.value, "//") == 0) {
        Token comment_skip;
        int current_running_line = current_line;
        while ((comment_skip = get_next_token(file)).type != T_EOF) {
            if (current_line != current_running_line) {
                parse_statement(file, comment_skip);
                break;
            }
        }
        return;
    }

    // ======================================================
    // 🌟 [v0.1.5 CORE FIX]: இஃப்-எல்ஸ் டோக்கன் பவுண்டரி மற்றும் பிரான்ச் கண்ட்ரோல் பிக்ஸ்!
    // ======================================================
    if (strcmp(t.value, "if") == 0 || strcmp(t.value, "எனர்") == 0 || strcmp(t.value, "எனில்") == 0) {
        Token open_p = get_next_token(file);  // '(' குறியீட்டை உட்கொள்கிறது
        Token v1 = get_next_token(file);      // முதல் மாறி அல்லது எண்
        Token op = get_next_token(file);      // ரிலேஷனல் ஆபரேட்டர் (<, >, ==, !=)
        Token v2 = get_next_token(file);      // இரண்டாம் மாறி அல்லது எண்
        Token close_p = get_next_token(file); // ')' குறியீட்டை உட்கொள்கிறது
        Token open_b = get_next_token(file);  // '{' குறியீட்டை உட்கொள்கிறது

        // codegen லேயரில் LLVM Branch கிராஃப்-ஐ துவக்க ஃபங்ஷனை அழைக்கிறோம் பிரபா
        extern void tamizhi_gen_if_start(char* lhs, char* rel_op, char* rhs);
        tamizhi_gen_if_start(v1.value, op.value, v2.value);

        // [TRUE BLOCK PARSING]: இஃப் பாடி பிளாக்கிற்குள் இருக்கும் ஸ்டேட்மென்ட்களை இயக்குகிறோம்
        int brace_count = 1;
        Token if_body;
        while (brace_count > 0 && (if_body = get_next_token(file)).type != T_EOF) {
            if (strcmp(if_body.value, "{") == 0) brace_count++;
            else if (strcmp(if_body.value, "}") == 0) {
                brace_count--;
                if (brace_count <= 0) break;
            }
            else parse_statement(file, if_body);
        }

        // 🌟 [CRITICAL]: ட்ரூ பிளாக் பார்சிங் முடிஞ்ச உடனே, எல்ஸ் செக் பண்றதுக்கு முன்னாடியே 
        // இந்த ட்ரூ பிளாக்கோட எண்டைக் குறிக்க codegen-க்கு சிக்னல் தர்றோம் பிரபா!
        extern void tamizhi_gen_if_body_end();
        tamizhi_gen_if_body_end();

        // அடுத்து 'else' அல்லது 'இல்லையெனில்' பிளாக் வருகிறதா என்று பார்க்க பேக்-அப் பொசிஷன்
        long backup_pos = ftell(file);
        Token next_tok = get_next_token(file);

        if (strcmp(next_tok.value, "else") == 0 || strcmp(next_tok.value, "இல்லையெனில்") == 0) {
            Token else_open_b = get_next_token(file); // எல்ஸ் பிளாக்கோட '{'

            extern void tamizhi_gen_else_start();
            tamizhi_gen_else_start();

            brace_count = 1;
            Token else_body;
            while (brace_count > 0 && (else_body = get_next_token(file)).type != T_EOF) {
                if (strcmp(else_body.value, "{") == 0) brace_count++;
                else if (strcmp(else_body.value, "}") == 0) {
                    brace_count--;
                    if (brace_count <= 0) break;
                }
                else parse_statement(file, else_body);
            }
        } else {
            // எல்ஸ் பிளாக் இல்லையென்றால் ஃபைல் பாயிண்டரை பழைய பொசிஷனுக்கே ரீசெட் செய்கிறோம் பிரபா
            fseek(file, backup_pos, SEEK_SET);
        }

        // கண்டிஷனல் பிரான்சிங் பிளாக்கின் முடிவைக் குறிக்கிறோம்
        extern void tamizhi_gen_if_end();
        tamizhi_gen_if_end();
        return;
    }

    // ======================================================
    // Number Variable Declaration
    // ======================================================

    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {

        Token name = get_next_token(file);
        Token next = get_next_token(file);

        while (next.type != 20 && next.type != T_EOF) {
            next = get_next_token(file);
        }

        long value_pos = ftell(file);
        Token value = get_next_token(file);
        tamizhi_trim_token(value.value);

        Token next_after_val = get_next_token(file);

        if (next_after_val.type == 15 || strcmp(next_after_val.value, "(") == 0) {

            long func_pos = find_function(value.value);
            if (func_pos != -1L) {
                while (get_next_token(file).type != 21);
                long return_pos = ftell(file);

                LLVMValueRef old_func = current_function;
                current_function = LLVMGetNamedFunction(module, value.value);

                call_depth++;
                fseek(file, func_pos, SEEK_SET);
                int brace_count = 1;
                Token body;

                while (brace_count > 0 && (body = get_next_token(file)).type != T_EOF) {
                    if (body.type == 22 || strcmp(body.value, "{") == 0) brace_count++;
                    else if (body.type == 23 || strcmp(body.value, "}") == 0) {
                        brace_count--;
                        if (brace_count <= 0) break;
                    }
                    else parse_statement(file, body);
                }

                call_depth--;

                current_function = old_func;
                fseek(file, return_pos, SEEK_SET);

                extern void tamizhi_gen_assign_from_return(char* var_name);
                tamizhi_gen_assign_from_return(name.value);

            } else {
                fprintf(stderr, "[Linker Error] Undefined Function '%s'\n", value.value);
            }
        }
        else {
            fseek(file, value_pos, SEEK_SET);
            Token current_tok = get_next_token(file);

            ASTNode* root = parse_expression(file, &current_tok);
            if (root) {
                extern void tamizhi_gen_math_ast(char* res_name, ASTNode* root);
                tamizhi_gen_math_ast(name.value, root);
                free_ast(root);
            } else {
                fprintf(stderr, "[Syntax Error] Invalid Number Expression for '%s'\n", name.value);
            }

            skip_remaining_if_needed(file, current_tok);
        }
    }

    // ======================================================
    // String Variable Declaration
    // ======================================================

    else if (t.type == T_STR || strcmp(t.value, "Str") == 0 || strcmp(t.value, "வரி") == 0) {

        Token name = get_next_token(file);
        Token next = get_next_token(file);

        while (next.type != 20 && next.type != T_EOF) {
            next = get_next_token(file);
        }

        Token value = get_next_token(file);

        char clean_str_val[1024];
        memset(clean_str_val, 0, sizeof(clean_str_val));

        int len = strlen(value.value);

        if (len == 0) {
            fprintf(stderr, "[Syntax Error] Empty string value for '%s'\n", name.value);
            return;
        }

        if (value.value[0] == '"' && value.value[len - 1] == '"' && len >= 2) {
            strncpy(clean_str_val, value.value + 1, len - 2);
            clean_str_val[len - 2] = '\0';
        }
        else {
            strcpy(clean_str_val, value.value);
        }

        tamizhi_trim_token(clean_str_val);
        tamizhi_gen_str(name.value, clean_str_val);
    }

    // ======================================================
    // Assignment Block
    // ======================================================

    else if (t.type == T_ID) {

        char var_name[100];
        strcpy(var_name, t.value);

        long current_pos = ftell(file);
        Token next = get_next_token(file);

        if (next.type == 20 || strcmp(next.value, "=") == 0) {

            long value_pos = ftell(file);
            Token current_tok = get_next_token(file);
            Token next_after_val = get_next_token(file);

            if (next_after_val.type == 15 || strcmp(next_after_val.value, "(") == 0) {

                long func_pos = find_function(current_tok.value);
                if (func_pos != -1L) {
                    while (get_next_token(file).type != 21);
                    long return_pos = ftell(file);

                    LLVMValueRef old_func = current_function;
                    current_function = LLVMGetNamedFunction(module, current_tok.value);

                    call_depth++;
                    fseek(file, func_pos, SEEK_SET);
                    int brace_count = 1;
                    Token body;

                    while (brace_count > 0 && (body = get_next_token(file)).type != T_EOF) {
                        if (body.type == 22 || strcmp(body.value, "{") == 0) brace_count++;
                        else if (body.type == 23 || strcmp(body.value, "}") == 0) {
                            brace_count--;
                            if (brace_count <= 0) break;
                        }
                        else parse_statement(file, body);
                    }

                    call_depth--;
                    current_function = old_func;
                    fseek(file, return_pos, SEEK_SET);

                    extern void tamizhi_gen_assign_from_return(char* var_name);
                    tamizhi_gen_assign_from_return(var_name);

                } else {
                    fprintf(stderr, "[Linker Error] Undefined Function '%s'\n", current_tok.value);
                }
            }
            else {
                fseek(file, value_pos, SEEK_SET);
                current_tok = get_next_token(file);
                ASTNode* root = parse_expression(file, &current_tok);

                if (root) {
                    extern void tamizhi_gen_math_ast(char* res_name, ASTNode* root);
                    tamizhi_gen_math_ast(var_name, root);
                    free_ast(root);
                }
                else {
                    fprintf(stderr, "[Syntax Error] Invalid Expression for '%s'\n", var_name);
                }

                skip_remaining_if_needed(file, current_tok);
            }
        }

        // ==============================================
        // Direct Function Call Block
        // ==============================================

        else if (next.type == 15 || strcmp(next.value, "(") == 0) {

            if (call_depth >= MAX_CALL_DEPTH) {
                fprintf(stderr, "[Runtime Error] Stack Overflow — Max call depth %d reached\n", MAX_CALL_DEPTH);
                return;
            }

            long func_pos = find_function(var_name);

            if (func_pos == -1L) {
                fprintf(stderr, "[Linker Error] Undefined Function '%s'\n", var_name);
                return;
            }

            while (get_next_token(file).type != 21);

            long return_pos = ftell(file);

            LLVMValueRef old_func = current_function;
            current_function = LLVMGetNamedFunction(module, var_name);

            call_depth++;

            fseek(file, func_pos, SEEK_SET);
            int brace_count = 1;
            Token body;

            while (brace_count > 0 && (body = get_next_token(file)).type != T_EOF) {

                if (body.type == 22 || strcmp(body.value, "{") == 0) {
                    brace_count++;
                    continue;
                }

                if (body.type == 23 || strcmp(body.value, "}") == 0) {
                    brace_count--;
                    if (brace_count <= 0) break;
                    continue;
                }

                parse_statement(file, body);
            }

            call_depth--;
            current_function = old_func;
            fseek(file, return_pos, SEEK_SET);
        }

        else {
            fseek(file, current_pos, SEEK_SET);
        }
    }

    // ======================================================
    // Print Block (With Advanced BODMAS AST Expression Support 🚀)
    // ======================================================

    else if (t.type == T_PRINT || strcmp(t.value, "அச்சிடு") == 0) {

        long current_pos = ftell(file);
        Token current_tok = get_next_token(file);

        if (current_tok.type == 15 || strcmp(current_tok.value, "(") == 0) {
            current_tok = get_next_token(file);
        }

        if (current_tok.value[0] == '"' || current_tok.type == T_STR || strchr(current_tok.value, '"') != NULL) {
            tamizhi_trim_token(current_tok.value);
            extern void tamizhi_gen_print(char* name);
            tamizhi_gen_print(current_tok.value);

            Token semi = get_next_token(file);
            skip_remaining_if_needed(file, semi);
        }
        else {
            fseek(file, current_pos, SEEK_SET);
            current_tok = get_next_token(file);
            if (current_tok.type == 15 || strcmp(current_tok.value, "(") == 0) {
                current_tok = get_next_token(file);
            }

            ASTNode* root = parse_expression(file, &current_tok);
            if (root) {
                extern void tamizhi_gen_math_ast(char* res_name, ASTNode* root);
                tamizhi_gen_math_ast("__tamizhi_print_tmp", root);
                extern void tamizhi_gen_print(char* name);
                tamizhi_gen_print("__tamizhi_print_tmp");
                free_ast(root);
            } else {
                fprintf(stderr, "[Syntax Error] Invalid Print Expression\n");
            }

            skip_remaining_if_needed(file, current_tok);
        }
    }

    // ======================================================
    // Return Block
    // ======================================================

    else if (t.type == T_RET || strcmp(t.value, "return") == 0 || strcmp(t.value, "திரும்பு") == 0) {

        Token expr_token = get_next_token(file);

        extern void tamizhi_gen_return(char* return_val);
        tamizhi_gen_return(expr_token.value);

        Token semi = get_next_token(file);
        skip_remaining_if_needed(file, semi);
        return;
    }

    // ======================================================
    // 🚀 [v0.1.5 ROBUST LOOP BOUNDARY FIX] - லூப் இன்பினைட் டோக்கன் மற்றும் கிராஷ் பிக்ஸ்!
    // ======================================================
    else if (t.type == T_FOR || strcmp(t.value, "சு") == 0) {

        Token limit_token = get_next_token(file);
        tamizhi_trim_token(limit_token.value); // டோக்கனில் இருக்கக்கூடிய செமிகோலன் மற்றும் ஸ்பேஸ்களை நீக்குகிறது

        int limit = 0;
        if (isdigit((unsigned char)limit_token.value[0]) ||
            (limit_token.value[0] == '-' && isdigit((unsigned char)limit_token.value[1]))) {
            limit = atoi(limit_token.value);
        } else if (limit_token.type == T_ID) {
            fprintf(stderr, "[Warning] Loop limit '%s' is a variable — only literal numbers supported currently\n", limit_token.value);
            limit = 0;
        } else {
            fprintf(stderr, "[Syntax Error] Invalid loop limit '%s'\n", limit_token.value);
            return;
        }

        if (limit <= 0) {
            fprintf(stderr, "[Warning] Loop limit is 0 or negative (%d) — skipping loop body\n", limit);
            Token skip;
            int depth = 0;
            while ((skip = get_next_token(file)).type != T_EOF) {
                if (skip.type == 22 || strcmp(skip.value, "{") == 0) depth++;
                else if (skip.type == 23 || strcmp(skip.value, "}") == 0) {
                    if (depth <= 1) break;
                    depth--;
                }
            }
            return;
        }

        Token next = get_next_token(file);

        // பக் தடுப்பு: ஒருவேளை லிமிட் டோக்கனுக்கு அடுத்து தவறுதலாக ';' இருந்தால் அதைத் தாண்டிச் செல்லும்
        if (strcmp(next.value, ";") == 0 || next.type == 21) {
            next = get_next_token(file);
        }

        if (next.type == 22 || strcmp(next.value, "{") == 0) {

            tamizhi_gen_loop_start(limit);

            Token body;
            int loop_brace_depth = 1; // நெஸ்டட் லூப்களை (Nested Loops) கச்சிதமாக டிராக் செய்ய டெப்த் டிராக்கர் பிரபா

            while (loop_brace_depth > 0 && (body = get_next_token(file)).type != T_EOF) {

                if (strcmp(body.value, "{") == 0 || body.type == 22) {
                    loop_brace_depth++;
                    continue;
                }

                if (body.type == 23 || strcmp(body.value, "}") == 0) {
                    loop_brace_depth--;
                    if (loop_brace_depth <= 0) break;
                    continue;
                }

                parse_statement(file, body);
            }

            tamizhi_gen_loop_end();
        } else {
            fprintf(stderr, "[Syntax Error] Expected '{' after loop limit but found '%s'\n", next.value);
        }
        return;
    }
}
