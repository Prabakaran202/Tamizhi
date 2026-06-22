#include "lexer.h"
#include "ast.h"
#include "codegen.h"
#include "codegen_bridge.h" // 🌟 நம்ம புதிய மாடுலர் பிரிட்ஜ் ஹெட்டர் இணைக்கப்பட்டது

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void tamizhi_gen_math_op(char* res_name, char* var1, char* op, char* var2);
void tamizhi_gen_print(char* var_name);
void tamizhi_gen_str(char* name, char* value);

// 🌟 Lexer Reset Function (lexer.c-ல் இருந்து வருகிறது)
extern void tamizhi_reset_lexer();

extern int current_line;
extern LLVMValueRef current_function; 
extern LLVMModuleRef module;          
extern LLVMContextRef context;         

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
    while (len > 0 && (isspace((unsigned char)str[len - 1]) || str[len - 1] == ';')) {
        str[len - 1] = '\0';
        len--;
    }
}

void skip_to_semicolon(FILE *file) {
    Token t;
    while ((t = get_next_token(file)).type != 21 && t.type != T_EOF);
}

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
    if (current_tok->type == T_NUM || isdigit((unsigned char)current_tok->value[0]) ||
        (current_tok->value[0] == '-' && isdigit((unsigned char)current_tok->value[1]))) {
        ASTNode* node = create_number_node(atoi(current_tok->value));
        *current_tok = get_next_token(file);
        return node;
    }
    if (current_tok->type == T_ID) {
        ASTNode* node = create_identifier_node(current_tok->value);
        *current_tok = get_next_token(file);
        return node;
    }
    return NULL;
}

ASTNode* parse_term(FILE* file, Token* current_tok) {
    ASTNode* left = parse_primary(file, current_tok);
    while (strcmp(current_tok->value, "*") == 0 || strcmp(current_tok->value, "/") == 0) {
        char op[10];
        strcpy(op, current_tok->value);
        *current_tok = get_next_token(file);
        ASTNode* right = parse_primary(file, current_tok);
        if (!right) return NULL;
        left = create_binop_node(op, left, right);
    }
    return left;
}

ASTNode* parse_expression(FILE* file, Token* current_tok) {
    ASTNode* left = parse_term(file, current_tok);
    while (strcmp(current_tok->value, "+") == 0 || strcmp(current_tok->value, "-") == 0) {
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
// 🌟 Function Table (API ARGUMENTS ENABLED)
// ==========================================================
typedef struct {
    char name[100];
    long pos;
    int arg_count;                  // ஃபங்ஷனில் எத்தனை ஆர்கியுமெண்ட்ஸ் உள்ளன?
    char arg_names[10][50];         // ஆர்கியுமெண்டின் பெயர் (எ.கா: a, b)
    char arg_types[10][10];         // ஆர்கியுமெண்டின் வகை (எ.கா: Num, Str)
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
// 🌟 Header Scan (SMART ARGUMENT PARSER)
// ==========================================================
void scan_headers(FILE *file) {
    Token t;
    rewind(file);
    function_count = 0;

    if (tamizhi_debug_mode) {
        fprintf(stderr, " -> Starting Header Pre-Scan...\n");
    }

    while ((t = get_next_token(file)).type != T_EOF) {
        if (strcmp(t.value, "fun") == 0 || strcmp(t.value, "நிகழ்") == 0 || t.type == T_FUNC) {
            Token name = get_next_token(file);
            if (!is_valid(name)) continue;

            char clean_name[100];
            strcpy(clean_name, name.value);
            char* bracket = strchr(clean_name, '(');
            if (bracket) *bracket = '\0';

            strcpy(functions[function_count].name, clean_name);
            functions[function_count].arg_count = 0;

            // 🌟 பிராக்கெட்டுக்குள் இருக்கும் ஆர்கியுமெண்ட்ஸ்களைப் படிக்கும் லாஜிக்
            Token tk = get_next_token(file);
            while (strcmp(tk.value, ")") != 0 && tk.type != 16 && tk.type != 22 && strcmp(tk.value, "{") != 0 && tk.type != T_EOF) {
                if (strcmp(tk.value, "Num") == 0 || strcmp(tk.value, "Str") == 0 || strcmp(tk.value, "எண்") == 0 || strcmp(tk.value, "வரி") == 0) {
                    strcpy(functions[function_count].arg_types[functions[function_count].arg_count], tk.value);
                    Token arg_name = get_next_token(file);
                    strcpy(functions[function_count].arg_names[functions[function_count].arg_count], arg_name.value);
                    functions[function_count].arg_count++;
                }
                tk = get_next_token(file);
            }

            // '{' வரும் வரை தேடி ஸ்கிப் செய்தல்
            while (tk.type != 22 && strcmp(tk.value, "{") != 0 && tk.type != T_EOF) {
                tk = get_next_token(file);
            }

            if (tk.type == 22 || strcmp(tk.value, "{") == 0) {
                functions[function_count].pos = ftell(file);
                
                if (tamizhi_debug_mode) {
                    fprintf(stderr, "    [Header] Registered: %s | Args: %d\n", clean_name, functions[function_count].arg_count);  
                }
                function_count++;

                int brace_depth = 1;
                Token skip_tk;
                while (brace_depth > 0 && (skip_tk = get_next_token(file)).type != T_EOF) {
                    if (strcmp(skip_tk.value, "{") == 0 || skip_tk.type == 22) brace_depth++;
                    if (strcmp(skip_tk.value, "}") == 0 || skip_tk.type == 23) brace_depth--;
                }
            }
        }
    }
    rewind(file);
    tamizhi_reset_lexer(); 
}

// ==========================================================
// Main Parse Entry
// ==========================================================
void parse(FILE *file) {
    Token t;
    long main_pos = -1L;
    long footer_pos = -1L;
    if (tamizhi_debug_mode) { 
        fprintf(stderr, "\n[Parser] --- Tamizhi Engine Started ---\n");
    }
    scan_headers(file);

    long check_main = find_function("main");
    if (check_main != -1L) main_pos = check_main;
    else main_pos = find_function("முதன்மை");

    long check_footer = find_function("footer");
    if (check_footer != -1L) footer_pos = check_footer;
    else footer_pos = find_function("பூட்டர்");

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
        if (strcmp(check_brace.value, "{") != 0) fseek(file, main_pos, SEEK_SET);

        current_function = LLVMGetNamedFunction(module, "main");

        int brace_count = 1;
        while (brace_count > 0) {
            t = get_next_token(file);
            if (t.type == T_EOF) break;
            if (t.type == 22 || strcmp(t.value, "{") == 0) { brace_count++; continue; }
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
        if (strcmp(check_brace.value, "{") != 0) fseek(file, footer_pos, SEEK_SET);

        int brace_count = 1;
        while (brace_count > 0) {
            t = get_next_token(file);
            if (t.type == T_EOF) break;
            if (t.type == 22 || strcmp(t.value, "{") == 0) { brace_count++; continue; }
            if (t.type == 23 || strcmp(t.value, "}") == 0) {
                brace_count--;
                if (brace_count <= 0) break;
                continue;
            }
            parse_statement(file, t);
        }
    }
    if (tamizhi_debug_mode) { fprintf(stderr, "[Parser] --- Completed Successfully ---\n"); }
}

// ==========================================================
// Statement Parser System
// ==========================================================
void parse_statement(FILE *file, Token t) {
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

    if (strcmp(t.value, "if") == 0 || strcmp(t.value, "எனர்") == 0 || strcmp(t.value, "எனில்") == 0) {
        Token tok = get_next_token(file);
        int has_paren = 0;

        if (strcmp(tok.value, "(") == 0 || tok.type == 15) {
            has_paren = 1;
            tok = get_next_token(file);
        }

        Token v1 = tok;                         // எ.கா: 'i'
        Token op = get_next_token(file);        // எ.கா: '=='
        Token v2 = get_next_token(file);        // எ.கா: '1'

        Token brace_tok = get_next_token(file);

        if (has_paren && (strcmp(brace_tok.value, ")") == 0 || brace_tok.type == 16)) {
            brace_tok = get_next_token(file);   // '{' ஐ வாங்குகிறோம்
        }

        tamizhi_gen_if_start(v1.value, op.value, v2.value);

        int brace_count = 1; 
        Token if_body;
        while (brace_count > 0 && (if_body = get_next_token(file)).type != T_EOF) {
            if (strcmp(if_body.value, "{") == 0 || if_body.type == 22) brace_count++;
            else if (strcmp(if_body.value, "}") == 0 || if_body.type == 23) {
                brace_count--;
                if (brace_count <= 0) break;
            }
            else parse_statement(file, if_body);
        }

        tamizhi_gen_if_body_end();

        long backup_pos = ftell(file);
        Token next_tok = get_next_token(file);

        if (strcmp(next_tok.value, "else") == 0 || strcmp(next_tok.value, "இல்லையெனில்") == 0) {
            get_next_token(file); 
            tamizhi_gen_else_start();

            brace_count = 1;
            Token else_body;
            while (brace_count > 0 && (else_body = get_next_token(file)).type != T_EOF) {
                if (strcmp(else_body.value, "{") == 0 || else_body.type == 22) brace_count++;
                else if (strcmp(else_body.value, "}") == 0 || else_body.type == 23) {
                    brace_count--;
                    if (brace_count <= 0) break;
                }
                else parse_statement(file, else_body);
            }
        } else {
            fseek(file, backup_pos, SEEK_SET);
        }

        tamizhi_gen_if_end();
        return;
    }

    if (strcmp(t.value, "இயக்கு") == 0||strcmp(t.value, "call") == 0 || t.type == T_SYSTEM) {
        Token cmd_token = get_next_token(file); 
        tamizhi_gen_system_call(cmd_token.value);

        Token semi = get_next_token(file);
        skip_remaining_if_needed(file, semi);
        return;
    }

    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {
        Token name = get_next_token(file);
        Token next = get_next_token(file);

        while (next.type != 20 && next.type != T_EOF) { next = get_next_token(file); }

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
                tamizhi_gen_math_ast(name.value, root);
                free_ast(root);
            } else {
                fprintf(stderr, "[Syntax Error] Invalid Number Expression for '%s'\n", name.value);
            }
            skip_remaining_if_needed(file, current_tok);
        }
    }

    else if (t.type == T_STR || strcmp(t.value, "Str") == 0 || strcmp(t.value, "வரி") == 0) {
        Token name = get_next_token(file);
        Token next = get_next_token(file);
        while (next.type != 20 && next.type != T_EOF) { next = get_next_token(file); }
        Token value = get_next_token(file);
        char clean_str_val[1024] = {0};
        int len = strlen(value.value);
        if (value.value[0] == '"' && value.value[len - 1] == '"' && len >= 2) {
            strncpy(clean_str_val, value.value + 1, len - 2);
        } else {
            strcpy(clean_str_val, value.value);
        }
        tamizhi_trim_token(clean_str_val);
        tamizhi_gen_str(name.value, clean_str_val);
    }

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
                    int brace_count = 1; Token body;
                    while (brace_count > 0 && (body = get_next_token(file)).type != T_EOF) {
                        if (body.type == 22 || strcmp(body.value, "{") == 0) brace_count++;
                        else if (body.type == 23 || strcmp(body.value, "}") == 0) { brace_count--; if (brace_count <= 0) break; }
                        else parse_statement(file, body);
                    }
                    call_depth--; current_function = old_func; fseek(file, return_pos, SEEK_SET);
                    tamizhi_gen_assign_from_return(var_name);
                }
            } else {
                fseek(file, value_pos, SEEK_SET);
                current_tok = get_next_token(file);
                ASTNode* root = parse_expression(file, &current_tok);
                if (root) {
                    tamizhi_gen_math_ast(var_name, root);
                    free_ast(root);
                }
                skip_remaining_if_needed(file, current_tok);
            }
        }
        else if (next.type == 15 || strcmp(next.value, "(") == 0) {
            long func_pos = find_function(var_name);
            if (func_pos != -1L) {
                while (get_next_token(file).type != 21);
                long return_pos = ftell(file);
                LLVMValueRef old_func = current_function;
                current_function = LLVMGetNamedFunction(module, var_name);
                fseek(file, func_pos, SEEK_SET);
                int brace_count = 1; Token body;
                while (brace_count > 0 && (body = get_next_token(file)).type != T_EOF) {
                    if (body.type == 22 || strcmp(body.value, "{") == 0) brace_count++;
                    else if (body.type == 23 || strcmp(body.value, "}") == 0) { brace_count--; if (brace_count <= 0) break; }
                    else parse_statement(file, body);
                }
                current_function = old_func; fseek(file, return_pos, SEEK_SET);
            }
        } else { fseek(file, current_pos, SEEK_SET); }
    }

    else if (t.type == T_PRINT || strcmp(t.value, "அச்சிடு") == 0) {
        long current_pos = ftell(file);
        Token current_tok = get_next_token(file);
        if (current_tok.type == 15 || strcmp(current_tok.value, "(") == 0) { current_tok = get_next_token(file); }

        if (current_tok.value[0] == '"' || current_tok.type == T_STR || strchr(current_tok.value, '"') != NULL) {
            tamizhi_trim_token(current_tok.value);
            tamizhi_gen_print(current_tok.value);
            Token semi = get_next_token(file); skip_remaining_if_needed(file, semi);
        } else {
            long backup_peek = ftell(file);
            Token peek = get_next_token(file);
            
            if (peek.type == 21 || strcmp(peek.value, ";") == 0 || peek.type == 16 || strcmp(peek.value, ")") == 0) {
                tamizhi_trim_token(current_tok.value);
                tamizhi_gen_print(current_tok.value);
            } else {
                fseek(file, current_pos, SEEK_SET);
                current_tok = get_next_token(file);
                if (current_tok.type == 15 || strcmp(current_tok.value, "(") == 0) { current_tok = get_next_token(file); }
                ASTNode* root = parse_expression(file, &current_tok);
                if (root) {
                    tamizhi_gen_math_ast("__tamizhi_print_tmp", root);
                    tamizhi_gen_print("__tamizhi_print_tmp");
                    free_ast(root);
                }
                skip_remaining_if_needed(file, current_tok);
            }
        }
    }

    else if (t.type == T_RET || strcmp(t.value, "return") == 0 || strcmp(t.value, "திரும்பு") == 0) {
        Token expr_token = get_next_token(file);
        tamizhi_gen_return(expr_token.value);
        Token semi = get_next_token(file); skip_remaining_if_needed(file, semi);
        return;
    }

    else if (t.type == T_FOR || strcmp(t.value, "சு") == 0) {
        Token limit_token = get_next_token(file);
        tamizhi_trim_token(limit_token.value);

        int limit = atoi(limit_token.value);
        Token next = get_next_token(file);

        if (strcmp(next.value, ";") == 0 || next.type == 21) { next = get_next_token(file); }

        if (next.type == 22 || strcmp(next.value, "{") == 0) {
            tamizhi_gen_loop_start(limit);

            Token body;
            int loop_brace_depth = 1;
            while (loop_brace_depth > 0 && (body = get_next_token(file)).type != T_EOF) {
                if (strcmp(body.value, "{") == 0 || body.type == 22) { loop_brace_depth++; continue; }
                if (body.type == 23 || strcmp(body.value, "}") == 0) {
                    loop_brace_depth--;
                    if (loop_brace_depth <= 0) break;
                    continue;
                }
                parse_statement(file, body);
            }
            tamizhi_gen_loop_end();
        }
        return;
    }
}
