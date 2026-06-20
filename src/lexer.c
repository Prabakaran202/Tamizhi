#include "lexer.h"
#include <string.h>
#include <ctype.h>

// 🌟 உலகளாவிய வரி எண் மாறி (Global Line Counter)
int current_line = 1;
// ==========================================================
// 🌟 EOF-ஐ ரீசெட் செய்யும் ஃபங்ஷன்
// ==========================================================
extern int current_char; // உங்கள் லெக்சரில் character-ஐ படிக்கும் வேரியபிள் பெயர் (LastChar என்று இருந்தால் அதைப் பயன்படுத்தவும்)

void tamizhi_reset_lexer() {
    current_char = ' '; // EOF தடையை உடைத்து ஸ்பேஸாக மாற்றுகிறோம்
}


T_Type get_keyword_type(char* value) {
    // 1. அமைப்பு / Structure
    if (strcmp(value, "முதன்மை") == 0 || strcmp(value, "main") == 0) return T_MAIN;
    if (strcmp(value, "நிகழ்") == 0 || strcmp(value, "fun") == 0) return T_FUNC;
    if (strcmp(value, "பூட்டர்") == 0 || strcmp(value, "footer") == 0) return T_FOOTER;

    // 2. வெளியீடு & உள்ளீடு / I/O
    if (strcmp(value, "அச்சிடு") == 0 || strcmp(value, "print") == 0) return T_PRINT;
    if (strcmp(value, "உள்ளீடு") == 0 || strcmp(value, "input") == 0) return T_INP;

    // 3. தரவு வகைகள் / Data Types
    if (strcmp(value, "எண்") == 0 || strcmp(value, "முழுஎண்") == 0 || strcmp(value, "Num") == 0) return T_INT;
    if (strcmp(value, "வரி") == 0 || strcmp(value, "Str") == 0) return T_STR;
    if (strcmp(value, "உண்மை") == 0 || strcmp(value, "bool") == 0) return T_BOOL;

    // 4. கட்டுப்பாட்டு லாஜிக் / Logic Control
    if (strcmp(value, "எனில்") == 0 || strcmp(value, "if") == 0) return T_IF;
    if (strcmp(value, "இல்லையெனில்") == 0 || strcmp(value, "else") == 0) return T_ELSE;
    if (strcmp(value, "சு") == 0 || strcmp(value, "for") == 0) return T_FOR;
    if (strcmp(value, "சு2") == 0 || strcmp(value, "while") == 0) return T_WHILE;

    // 5. இதர / Others
    if (strcmp(value, "திரும்பு") == 0 || strcmp(value, "return") == 0) return T_RET;
    if (strcmp(value, "இயக்கு") == 0 || strcmp(value, "call") == 0) return T_CALL;
    if (strcmp(value, "வரிசை") == 0 || strcmp(value, "line") == 0) return T_LINE;

    return T_ID; 
}

Token get_next_token(FILE *file) {
    Token token;
    int c = fgetc(file);

    // 🌟 ஒயிட்ஸ்பேஸ்களை கடக்கும்போது புதிய வரிகளை துல்லியமாக கணக்கிடும் வளையம்
    while (isspace(c)) {
        if (c == '\n') {
            current_line++;
        }
        c = fgetc(file);
    }

    // 🌟 மாஸ்டர் பிக்ஸ்: சிங்கிள் லைன் கமெண்ட்களை (//) அப்படியே புறக்கணிக்கும் பாதுகாப்பு லேயர்
    if (c == '/') {
        int next_c = fgetc(file);
        if (next_c == '/') {
            while ((c = fgetc(file)) != '\n' && c != EOF);
            if (c == '\n') {
                current_line++;
            }
            return get_next_token(file);
        } else {
            ungetc(next_c, file);
        }
    }

    // தற்போதைய டோக்கன் எந்த வரியில் இருக்கிறது என்பதை டோக்கன் ஸ்ட்ரக்சரில் லாக் செய்கிறோம்
    token.line = current_line;

    if (c == EOF) {
        token.type = T_EOF;
        strcpy(token.value, "EOF");
        return token;
    }

    // =========================================================================
    // 🧵 [v0.1.5 REFACTORED STRING LEXER FIX]: டபுள் கோட்ஸ் உள்ளே இருக்கும் அத்தனையையும் உடைக்காமல் பியூர் ஸ்ட்ரிங்காக மாற்றும் மாஸ்டர் லாஜிக் பிரபா!
    // =========================================================================
    if (c == '"') {
        token.type = T_STR;
        int i = 0;
        
        // ஆரம்ப டபுள் கோட்ஸையும் டோக்கன் வேல்யூவோடு சேர்க்கிறோம் பிரபா
        token.value[i++] = c; 

        // அடுத்த டபுள் கோட்ஸ் அல்லது ஃபைல் எண்ட் வர்ற வரைக்கும் எல்லா கேரக்டரையும் அப்படியே அள்ளுகிறோம்
        while ((c = fgetc(file)) != '"' && c != EOF) {
            if (c == '\n') current_line++; // ஸ்ட்ரிங்கிற்குள் புதிய வரி இருந்தாலும் கணக்கிடும்

            // பஃபர் பாதுகாப்பு அடுக்கு (1023 லிமிட் செக்)
            if (i < 1022) { 
                token.value[i++] = c;
            }
        }
        
        // இறுதி டபுள் கோட்ஸையும் உள்ளே லாக் பண்றோம் பிரபா
        if (c == '"') {
            token.value[i++] = '"';
        }
        
        token.value[i] = '\0';
        return token;
    }
    // =========================================================================

    // 🌟 மாஸ்டர் யூனிகோட் ஃபிக்ஸ்: தமிழ் மற்றும் ஆங்கில எழுத்துக்கள் (Identifiers / Keywords)
    if (isalpha(c) || (unsigned char)c >= 128 || c == '_') {
        int i = 0;
        do {
            if (i < 1023) {
                token.value[i++] = c;
            }
            c = fgetc(file);
        } while (isalpha(c) || isdigit(c) || (unsigned char)c >= 128 || c == '_'); 
        ungetc(c, file);
        token.value[i] = '\0';
        token.type = get_keyword_type(token.value);
        return token;
    }

    // எண்களைக் கையாளுதல் (Numbers)
    if (isdigit(c)) {
        int i = 0;
        while (isdigit(c)) {
            if (i < 1023) {
                token.value[i++] = c;
            }
            c = fgetc(file);
        }
        ungetc(c, file);
        token.value[i] = '\0';
        token.type = T_NUM;
        return token;
    }

    // குறியீடுகள் (Symbols)
    token.value[0] = c;
    token.value[1] = '\0';

    if (c == '(') token.type = 15;
    else if (c == ')') token.type = 16;
    else if (c == ';') token.type = 21; 
    else if (c == '<') token.type = 18;
    else if (c == '>') token.type = 24; 
    else if (c == '+') token.type = 19;
    else if (c == '-') token.type = 56; 
    else if (c == '=') token.type = 20;
    else if (c == '{') token.type = 22;
    else if (c == '}') token.type = 23;
    else token.type = T_ID;

    return token;
}
