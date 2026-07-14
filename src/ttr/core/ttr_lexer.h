#ifndef TTR_LEXER_H
#define TTR_LEXER_H

// பைத்தான் கோடில் நாம் கண்டுபிடிக்க வேண்டிய டோக்கன்கள்
typedef enum {
    TOKEN_PRINT,      // print
    TOKEN_STRING,     // "..." அல்லது '...'
    TOKEN_LPAREN,     // (
    TOKEN_RPAREN,
    TOKEN_IDENTIFIER, // name, age
    TOKEN_ASSIGN,     // =
    TOKEN_NUMBER,     
    TOKEN_EOF         // End of File
} TTR_TokenType;

// ஒரு டோக்கனின் அமைப்பு (Token என்பது TTR_Token என மாற்றப்பட்டுள்ளது)
typedef struct {
    TTR_TokenType type;
    char value[256];
} TTR_Token;

// Lexer ஃபங்ஷன்ஸ்
void ttr_init_lexer(const char *source_code);
TTR_Token ttr_get_next_token(); // ttr_get_next_token என மாற்றப்பட்டுள்ளது

#endif
