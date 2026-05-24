#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// 🌟 1. மெமரியில் புதிய நோடை உருவாக்கும் அடிப்படை ஹெல்ப்பர் ஃபங்க்ஷன்
ASTNode* create_node(ASTNodeType type) {
    // சிஸ்டம் ரேமில் (RAM) இந்த நோடுக்கான இடத்தை ஒதுக்குகிறோம்
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    
    if (node == NULL) {
        fprintf(stderr, "[Memory Error] AST மரம் உருவாக்க மெமரி போதவில்லை!\n");
        exit(1);
    }
    
    node->type = type;
    return node;
}

// 🌟 2. எண்களுக்கான கிளை (Number Node)
ASTNode* create_number_node(int value) {
    ASTNode* node = create_node(AST_NUMBER);
    node->data.num_value = value;
    return node;
}

// 🌟 3. மாறிகளுக்கான கிளை (Identifier Node)
ASTNode* create_identifier_node(char* name) {
    ASTNode* node = create_node(AST_IDENTIFIER);
    // பஃபர் ஓவர்ஃபுளோ வராமல் இருக்க strncpy பயன்படுத்துகிறோம்
    strncpy(node->data.var_name, name, 255);
    node->data.var_name[255] = '\0';
    return node;
}

// 🌟 4. கணித செயல்பாடுகளுக்கான கிளை (Binary Operation Node)
ASTNode* create_binop_node(char* op, ASTNode* left, ASTNode* right) {
    ASTNode* node = create_node(AST_BINARY_OP);
    strncpy(node->data.binop.op, op, 9);
    node->data.binop.op[9] = '\0';
    
    // இடது மற்றும் வலது கிளைகளை இணைக்கிறோம்
    node->data.binop.left = left;
    node->data.binop.right = right;
    
    return node;
}

