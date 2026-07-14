#ifndef TTR_LEXER_H
#define TTR_LEXER_H

// பைத்தான் கோடில் நாம் கண்டுபிடிக்க வேண்டிய டோக்கன்கள்
typedef enum {
    TOKEN_PRINT,      // print
    TOKEN_STRING,     // "..." அல்லது '...'
    TOKEN_LPAREN,     // (
    TOKEN_RPAREN,
    TOKEN_IDENTIFIER, // name, age (புதியது)
    TOKEN_ASSIGN,     // = (புதியது)
    TOKEN_NUMBER,       // )
    TOKEN_EOF         // End of File
} TTR_TokenType;

// ஒரு டோக்கனின் அமைப்பு
typedef struct {
    TTR_TokenType type;
    char value[256];
} Token;

// Lexer ஃபங்ஷன்ஸ்
void ttr_init_lexer(const char *source_code);
Token get_next_token();

#endif
