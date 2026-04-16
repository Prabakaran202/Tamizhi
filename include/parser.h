#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// AST Node vagaigal
typedef enum {
    NODE_VAR_DECL,  // Variable declaration (எண் அ = 100)
    NODE_PRINT,     // Output (கூறு)
    NODE_BINARY_OP  // Math operations (+, -, *, /)
} NodeType;

typedef struct ASTNode {
    NodeType type;
    Token token;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

void parse(FILE *file);

#endif
