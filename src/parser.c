#include "parser.h"
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
    
    // BACKEND LINK 🔥
                tamizhi_gen_print(p_name.value); 
            }

        // 3. 'சு' (Loop) handle pannuvom
         else if (strcmp(t.value, "சு") == 0) {
            fprintf(stderr,"[Parser] Loop detected! Triggering 1M Loop Test...\n");
            tamizhi_gen_loop_test(1000000);
        }
    }
}
