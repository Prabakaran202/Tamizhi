/*#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// AST Node create panna helper function
ASTNode* create_node(NodeType type, Token t) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->token = t;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void parse(FILE *file) {
    Token t;
    while ((t = get_next_token(file)).type != T_EOF) {

        // 1. 'எண்' (Variable Declaration) handle pannuvom
        if (t.type == 13) { 
            Token name = get_next_token(file);   // Variable name (e.g., 'அ')
            Token assign = get_next_token(file); // '='
            Token val = get_next_token(file);    // Value (e.g., '100')
            Token semi = get_next_token(file);   // ';'

            fprintf(stderr,"[Parser] Variable Declaration Detect: %s = %s\n", name.value, val.value);

            // AST Node creation (Good for structure)
            ASTNode* var_node = create_node(NODE_VAR_DECL, name);

            // 🔥 LINK TO BACKEND (Phone ASCII Fix)
            // 'val.value' inga thaan correct-a access aagum
            //tamizhi_gen_var_decl("v1", atoi(val.value));
            
            tamizhi_gen_var_decl(name.value, atoi(val.value));
            
        }

        // 2. 'கூறு' (Print Statement) handle pannuvom
            else if (t.type == 14) { // 14 thaan 'கூறு' token type-nu assume pannuvom
                Token open_p = get_next_token(file);
                Token p_name = get_next_token(file); // Variable name (e.g., "எண்ணி")
                Token close_p = get_next_token(file);
                Token p_semi = get_next_token(file);
                fprintf(stderr, "[Parser] Print Statement Detect: %s\n", p_name.value);
    
    
            }

        // 3. 'சு' (Loop) handle pannuvom
         else if (strcmp(t.value, "சு") == 0) {
            fprintf(stderr,"[Parser] Loop detected! Triggering 1M Loop Test...\n");
            tamizhi_gen_loop_test(1000000);
        }
    }
}
*/

#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Helper to check if the token is what we expect
void expect(T_Type expected_type, FILE *file) {
    Token t = get_next_token(file);
    if (t.type != expected_type) {
        fprintf(stderr, "[Error] Expected token type %d but got %s\n", expected_type, t.value);
    }
}

// 1. Math Expression Parse panna (e.g., 1 + 4)
void parse_expression(FILE *file, Token first_token) {
    Token op = get_next_token(file); // '+'
    Token second_val = get_next_token(file); // '4'
    
    if (strcmp(op.value, "+") == 0) {
        fprintf(stderr, "[Parser] Math: %s + %s\n", first_token.value, second_val.value);
        // Inga tamizhi_gen_add(atoi(first_token.value), atoi(second_val.value)) call aagum
    }
}

void parse(FILE *file) {
    Token t;
    while ((t = get_next_token(file)).type != T_EOF) {

        // --- HANDLE 'முதன்மை' (Main) ---
        if (t.type == T_MAIN) {
            expect(15, file); // Expect '('
            expect(16, file); // Expect ')'
            expect(22, file); // Expect '{'
            fprintf(stderr, "[Parser] Entering Main Block\n");
            // Main block content parsing logic here...
        }

        // --- HANDLE 'நிகழ்' (Function Definition) ---
        else if (t.type == T_FUNC) {
            Token func_name = get_next_token(file); // 'add'
            expect(15, file); // '('
            expect(16, file); // ')'
            expect(17, file); // ';' or ':' based on your image
            fprintf(stderr, "[Parser] Defining Function: %s\n", func_name.value);
        }

        // --- HANDLE 'அச்சிடு' (Print) with Expressions ---
        else if (t.type == T_PRINT) {
            expect(15, file); // '('
            Token first = get_next_token(file);
            
            // Check if it's a simple variable or an expression (a + b)
            Token next = get_next_token(file);
            if (strcmp(next.value, "+") == 0) {
                Token second = get_next_token(file);
                fprintf(stderr, "[Parser] Print Expression: %s + %s\n", first.value, second.value);
            }
            expect(16, file); // ')'
            expect(17, file); // ';'
        }

        // --- HANDLE 'இயக்கு' (Function Call/Execute) ---
        else if (t.type == T_CALL) {
            expect(15, file); // '('
            expect(22, file); // '{'
            
            Token func_to_call = get_next_token(file); // 'add'
            expect(15, file); // '('
            
            Token param = get_next_token(file); // '1'
            parse_expression(file, param); // Handle '1 + 4'
            
            expect(16, file); // ')'
            expect(17, file); // ';'
            expect(23, file); // '}'
            expect(17, file); // ';'
            fprintf(stderr, "[Parser] Calling Function %s with expression\n", func_to_call.value);
        }
    }
}
