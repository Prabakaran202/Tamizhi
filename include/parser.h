#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"

// AST Node வகைகள்
typedef enum {
    NODE_VAR_DECL,   
    NODE_PRINT,      
    NODE_BINARY_OP,  
    NODE_LOOP,
    NODE_FUNC_DEF,   // 'நிகழ்' -க்காக சேர்க்கப்பட்டது
    NODE_FUNC_CALL   // 'இயக்கு' -க்காக சேர்க்கப்பட்டது
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

// புதிய பங்க்ஷன்கள் (இவை Parser-க்குள் உள்முகமாகப் பயன்படும்)
void parse_expression(FILE *file, Token first_token);
void expect(int expected_type, FILE *file);

#endif
