#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

typedef enum {
    // தரவு வகைகள்
    T_INT, T_STR, T_FLT, T_BOOL,
    
    // கட்டுப்பாட்டு லாஜிக்
    T_IF, T_ELSE, T_FOR, T_WHILE,
    
    // செயல்பாடுகள்
    T_RET, T_FUNC, T_PRINT, T_INP, T_IMP, 
    
    // தமிழி ஸ்பெஷல் (இவைதான் விடுபட்டவை)
    T_MAIN, T_CALL, T_FOOTER, T_LINE,
    
    // பொதுவானவை
    T_ID, T_NUM, T_EOF
} T_Type;

typedef struct {
    T_Type type;
    char value[256]; // 64-லிருந்து 256-ஆக மாற்றியுள்ளேன், தமிழ் எழுத்துக்களுக்கு அதிக இடம் தேவைப்படும்.
} Token;

Token get_next_token(FILE *file);
T_Type get_keyword_type(char* value);

#endif
