#ifndef AST_H
#define AST_H

// 🌟 1. மரத்தின் கிளை வகைகள் (Node Types)
typedef enum {
    AST_NUMBER,       // எண்கள் (உதா: 5, 100)
    AST_IDENTIFIER,   // மாறிகளின் பெயர்கள் (உதா: முதல்_எண்)
    AST_BINARY_OP     // கணித செயல்பாடுகள் (+, -, *, /)
} ASTNodeType;

// 🌟 2. AST மரம் கட்டமைப்பு (The Tree Structure)
typedef struct ASTNode {
    ASTNodeType type; // இது என்ன வகையான நோட் என்று சொல்லும்
    
    // Union மூலம் மெமரியை மிச்சப்படுத்துகிறோம் (எந்த நோட் வகையோ அதற்கான டேட்டா மட்டும் இருக்கும்)
    union {
        // எண்களுக்கான டேட்டா
        int num_value;       
        
        // மாறிகளுக்கான டேட்டா (Variable Name)
        char var_name[256];  
        
        // கணித செயல்பாடுகளுக்கான டேட்டா (Binary Operation)
        struct {
            char op[10];           // ஆபரேட்டர் (+, -, *, /)
            struct ASTNode* left;  // இடது கிளை (Left Child)
            struct ASTNode* right; // வலது கிளை (Right Child)
        } binop;
    } data;
} ASTNode;

// 🌟 3. நோட்களை உருவாக்கும் ஃபங்க்ஷன்களின் முன்வரைவு (Declarations)
ASTNode* create_number_node(int value);
ASTNode* create_identifier_node(char* name);
ASTNode* create_binop_node(char* op, ASTNode* left, ASTNode* right);

#endif
