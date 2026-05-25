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

void skip_to_semicolon(FILE *file) {
    Token t;

    while (
        (t = get_next_token(file)).type != 21 &&
        t.type != T_EOF
    );
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

    // Parenthesis Support - ( எக்ஸ்பிரஷன்களை பிரித்து முன்னுரிமை அளிக்கிறது )
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
    rewind(file);

    while ((t = get_next_token(file)).type != T_EOF) {

        if (strcmp(t.value, "main") == 0 || strcmp(t.value, "முதன்மை") == 0) {

            Token brace;
            while ((brace = get_next_token(file)).type != T_EOF) {
                if (brace.type == 22 || strcmp(brace.value, "{") == 0) {
                    main_pos = ftell(file);
                    break;
                }
            }
        }

        else if (strcmp(t.value, "footer") == 0 || strcmp(t.value, "பூட்டர்") == 0) {

            Token brace;
            while ((brace = get_next_token(file)).type != T_EOF) {
                if (brace.type == 22 || strcmp(brace.value, "{") == 0) {
                    footer_pos = ftell(file);
                    break;
                }
            }
        }
    }

    tamizhi_generate_entry();

    // ======================================================
    // MAIN BLOCK RUN
    // ======================================================

    if (main_pos != -1L) {

        fseek(file, main_pos, SEEK_SET);
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

    // ======================================================
    // FOOTER BLOCK RUN
    // ======================================================

    if (footer_pos != -1L) {

        fseek(file, footer_pos, SEEK_SET);
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

    // ======================================================
    // Number Variable Declaration
    // ======================================================

    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {

        Token name = get_next_token(file);
        Token next = get_next_token(file);

        while (next.type != 20 && next.type != T_EOF) {
            next = get_next_token(file);
        }

        Token value = get_next_token(file);
        tamizhi_trim_token(value.value);

        if (isdigit((unsigned char)value.value[0]) || value.value[0] == '-') {
            tamizhi_gen_var(name.value, atoi(value.value));
        }
        else {
            fprintf(stderr, "[Syntax Error] Invalid Number '%s'\n", value.value);
        }
    }

    // ======================================================
    // String Variable Declaration (Fixed Quotes Strip)
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
    // Assignment + AST Math (BODMAS Sync Fixed 🚀)
    // ======================================================

    else if (t.type == T_ID) {

        char var_name[100];
        strcpy(var_name, t.value);

        long current_pos = ftell(file);
        Token next = get_next_token(file);

        // ==============================================
        // Assignment Block
        // ==============================================

        if (next.type == 20 || strcmp(next.value, "=") == 0) {

            Token current_tok = get_next_token(file);

            ASTNode* root = parse_expression(file, &current_tok);

            if (root) {
                extern void tamizhi_gen_math_ast(char* res_name, ASTNode* root);
                tamizhi_gen_math_ast(var_name, root);
                free_ast(root);
            }
            else {
                fprintf(stderr, "[Syntax Error] Invalid Expression\n");
            }

            if (strcmp(current_tok.value, ";") != 0 && current_tok.type != 21) {
                skip_to_semicolon(file);
            }
        }

        // ==============================================
        // Function Call Block
        // ==============================================

        else if (next.type == 15 || strcmp(next.value, "(") == 0) {

            if (call_depth >= MAX_CALL_DEPTH) {
                fprintf(stderr, "[Runtime Error] Stack Overflow\n");
                return;
            }

            long func_pos = find_function(var_name);

            if (func_pos == -1L) {
                fprintf(stderr, "[Linker Error] Undefined Function '%s'\n", var_name);
                return;
            }

            while (get_next_token(file).type != 21);

            long return_pos = ftell(file);
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
            fseek(file, return_pos, SEEK_SET);
        }

        else {
            fseek(file, current_pos, SEEK_SET);
        }
    }

    // ======================================================
    // Print Block (Fixed Trailing Semicolon Sync 🚀)
    // ======================================================

    else if (t.type == T_PRINT || strcmp(t.value, "அச்சிடு") == 0) {

        Token first = get_next_token(file);
        if (first.type == 15) first = get_next_token(file);

        tamizhi_trim_token(first.value);
        tamizhi_gen_print(first.value);

        // 🌟 பிக்ஸ்: பிரிண்ட் முடிந்ததும் அந்த வரியின் செமைகோலன் ';' டோக்கனை 
        // பைப்லைனில் இருந்து கிளியர் செய்து அடுத்த வரிக்கு பாயிண்டரை நகர்த்துகிறோம்!
        Token semi = get_next_token(file);
        if (strcmp(semi.value, ";") != 0 && semi.type != 21) {
            skip_to_semicolon(file);
        }
    }

    // ======================================================
    // Loop Block
    // ======================================================

    else if (t.type == T_FOR || strcmp(t.value, "சு") == 0) {

        Token limit_token = get_next_token(file);
        int limit = atoi(limit_token.value);

        Token next = get_next_token(file);

        if (next.type == 22 || strcmp(next.value, "{") == 0) {

            tamizhi_gen_loop_start(limit);
            Token body;

            while ((body = get_next_token(file)).type != T_EOF) {

                if (body.type == 23 || strcmp(body.value, "}") == 0)
                    break;

                parse_statement(file, body);
            }

            tamizhi_gen_loop_end();
        }
    }
}
