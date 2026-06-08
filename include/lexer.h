#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

// 🌟 தமிழி டோக்கன் வகைகள் (Merged & Complete)
typedef enum {
    // தரவு வகைகள்
    T_INT, 
    T_STR, 
    T_FLT,   // 🌟 பழைய கோப்பில் இருந்தது, இப்போ மீட்கப்பட்டது!
    T_BOOL,

    // கட்டுப்பாட்டு லாஜிக்
    T_IF, 
    T_ELSE, 
    T_FOR, 
    T_WHILE,T_SYSTEM, 

    // செயல்பாடுகள்
    T_RET, 
    T_FUNC, 
    T_PRINT, 
    T_INP, 
    T_IMP,   // 🌟 பழைய கோப்பில் இருந்தது, இப்போ மீட்கப்பட்டது!

    // தமிழி ஸ்பெஷல் 
    T_MAIN, 
    T_CALL, 
    T_FOOTER, 
    T_LINE,

    // பொதுவானவை
    T_ID, 
    T_NUM, 
    T_EOF
} T_Type;

// 🌟 டோக்கன் கட்டமைப்பு (Token Structure)
typedef struct {
    T_Type type;
    char value[1024]; // 🌟 தமிழ் எழுத்துக்களின் யூனிகோட் மெமரிக்காக பக்கா பாதுகாப்பு (1024)
    int line;         // 🌟 எரர் டிராக்கிங் செய்ய உதவும் மாஸ்டர் மெம்பர்!
} Token;

// பங்க்ஷன் பிரகடனங்கள்
Token get_next_token(FILE *file);
T_Type get_keyword_type(char* value);

#endif
