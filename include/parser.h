#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"

// AST Node வகைகள் (NodeType)
typedef enum {
    NODE_VAR_DECL,   
    NODE_PRINT,      
    NODE_BINARY_OP,  
    NODE_LOOP,
    NODE_FUNC_DEF,   
    NODE_FUNC_CALL   
} NodeType;

// AST Structure
typedef struct ASTNode {
    NodeType type;          
    Token token;            
    struct ASTNode *left;   
    struct ASTNode *right;  
} ASTNode;

// Function declarations
void parse(FILE *file);                      
ASTNode* create_node(NodeType type, Token t); 

// பழைய expect மற்றும் parse_expression வரிகளை இங்கே நீக்கிவிட்டோம் 
// ஏென்றால் அவை parser.c-ல் இப்போது இல்லை.

#endif
