#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Helper function to create a new AST node
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
        
        // 1. Handling 'எண்' (Variable Declaration)
        // Assume 13 is the token type for 'எண்' in your lexer
        if (t.type == 13) { 
            Token name = get_next_token(file);   // Get variable name
            Token assign = get_next_token(file); // Get '='
            Token val = get_next_token(file);    // Get value
            Token semi = get_next_token(file);   // Get ';'

            printf("[Parser] Variable Declaration Detect: %s = %s\n", name.value, val.value);

            // AST Node creation (Optional but good for future)
            ASTNode* var_node = create_node(NODE_VAR_DECL, name);
            
            // 🔥 LINK TO BACKEND (LLVM IR Generation)
            //tamizhi_gen_var_decl(name.value, atoi(val.value));
        }
        tamizhi_gen_var_decl("v1", atoi(val.value)); 
        // 2. Handling 'கூறு' (Print Statement)
        else if (t.type == 14) { // Assume 14 for 'கூறு'
            Token open_p = get_next_token(file); // '('
            Token name = get_next_token(file);   // variable name
            Token close_p = get_next_token(file); // ')'
            Token semi = get_next_token(file);    // ';'

            printf("[Parser] Print Statement Detect: %s\n", name.value);
            // Backend print logic night Arch-la add pannuvom
        }

        // 3. Handling 'சு' (Loop)
        else if (strcmp(t.value, "சு") == 0) {
            printf("[Parser] Loop detected! Triggering 1M Loop Test...\n");
            tamizhi_gen_loop_test(1000000);
        }
    }
}
