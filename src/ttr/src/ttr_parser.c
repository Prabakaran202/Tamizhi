#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/parser.h"

ASTProgram parse() {
    ASTProgram program;
    program.count = 0;

    Token current_token = get_next_token();

    // ஃபைல் முடியும் வரை (EOF) எல்லா வரிகளையும் படி!
    while (current_token.type != TOKEN_EOF && program.count < 100) {
        ASTNode node;
        node.type = -1;

        if (current_token.type == TOKEN_PRINT) {
            current_token = get_next_token(); // '('
            current_token = get_next_token(); // String or Identifier

            if (current_token.type == TOKEN_STRING) {
                node.type = AST_PRINT;
                strcpy(node.value, current_token.value);
            } else if (current_token.type == TOKEN_IDENTIFIER) {
                node.type = AST_PRINT_VAR; // Variable Print!
                strcpy(node.var_name, current_token.value);
            }

            current_token = get_next_token(); // ')'
            current_token = get_next_token(); // Move to next statement
        } 
        else if (current_token.type == TOKEN_IDENTIFIER) {
            strcpy(node.var_name, current_token.value);
            
            current_token = get_next_token(); // '='
            current_token = get_next_token(); // Value
            
            node.type = AST_ASSIGNMENT;
            if (current_token.type == TOKEN_NUMBER) {
                strcpy(node.var_type, "Num");
                strcpy(node.value, current_token.value);
            } else if (current_token.type == TOKEN_STRING) {
                strcpy(node.var_type, "Str");
                sprintf(node.value, "\"%s\"", current_token.value);
            }
            
            current_token = get_next_token(); // Move to next statement
        } else {
            current_token = get_next_token(); // Skip unknown
        }

        if (node.type != -1) {
            program.nodes[program.count++] = node;
        }
    }

    return program;
}
