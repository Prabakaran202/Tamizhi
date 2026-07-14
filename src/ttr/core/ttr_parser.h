#ifndef TTR_PARSER_H
#define TTR_PARSER_H

#include "ttr_lexer.h"

typedef enum {
    AST_PRINT,        // print("Hello")
    AST_PRINT_VAR,    // print(a) - புதியது!
    AST_ASSIGNMENT    // a = 12
} ASTNodeType;

typedef struct {
    ASTNodeType type;
    char value[256];
    char var_name[256];
    char var_type[10];
} ASTNode;

// பல வரிகளைச் சேமிக்க ஒரு Array (List of AST Nodes)
typedef struct {
    ASTNode nodes[100];
    int count;
} ASTProgram;

ASTProgram parse();

#endif
