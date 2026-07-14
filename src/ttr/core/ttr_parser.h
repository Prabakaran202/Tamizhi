#ifndef TTR_PARSER_H
#define TTR_PARSER_H

#include "ttr_lexer.h"

typedef enum {
    AST_PRINT,        // print("Hello")
    AST_PRINT_VAR,    // print(a) - புதியது!
    AST_ASSIGNMENT    // a = 12
} TTR_ASTNodeType;

typedef struct {
    TTR_ASTNodeType type;  // 🔥 இங்கே ASTNodeType என்று இருந்தது, அதை TTR_ASTNodeType என மாற்றியாச்சு!
    char value[256];
    char var_name[256];
    char var_type[10];
} TTR_ASTNode;

// பல வரிகளைச் சேமிக்க ஒரு Array (List of AST Nodes)
typedef struct {
    TTR_ASTNode nodes[100];
    int count;
} TTR_ASTProgram;

TTR_ASTProgram ttr_parse();

#endif
