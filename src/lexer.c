#include "lexer.h"
#include <string.h>
#include <ctype.h>

// 🌟 உலகளாவிய வரி எண் மாறி (Global Line Counter)
int current_line = 1;

// ==========================================================
// 🌟 EOF-ஐ ரீசெட் செய்யும் ஃபங்ஷன் (Dummy Function for Linker)
// ==========================================================
void tamizhi_reset_lexer() {
    // parser.c-ல் உள்ள rewind(file) தானாகவே EOF-ஐ க்ளியர் செய்துவிடுவதால்,
    // இங்கே எதுவும் செய்யத் தேவையில்லை. Linker Error வராமல் இருக்க மட்டும் இந்த ஃபங்ஷன் உள்ளது.
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

    token.line = current_line;

    if (c == EOF) {
        token.type = T_EOF;
        strcpy(token.value, "EOF");
        return token;
    }

    // =========================================================================
    // 🧵 String Lexer Fix
    // =========================================================================
    if (c == '"') {
        token.type = T_STR;
        int i = 0;
        token.value[i++] = c; 
        while ((c = fgetc(file)) != '"' && c != EOF) {
            if (c == '\n') current_line++; 
            if (i < 1022) { 
                token.value[i++] = c;
            }
        }
        if (c == '"') {
            token.value[i++] = '"';
        }
        token.value[i] = '\0';
        return token;
    }

    // 🌟 மாஸ்டர் யூனிகோட் ஃபிக்ஸ் (Identifiers / Keywords)
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

    // =========================================================================
    // 🌟 மாஸான பிக்ஸ்: குறியீடுகளை (==, !=, <=, >=) சரியாகப் பிரிக்கும் லாஜிக்
    // =========================================================================
    token.value[0] = c;
    token.value[1] = '\0';

    if (c == '=') {
        int next_c = fgetc(file);
        if (next_c == '=') {
            token.value[1] = '='; token.value[2] = '\0'; token.type = 30; // EQ
        } else {
            ungetc(next_c, file); token.type = 20; // ASSIGN
        }
    }
    else if (c == '!') {
        int next_c = fgetc(file);
        if (next_c == '=') {
            token.value[1] = '='; token.value[2] = '\0'; token.type = 31; // NEQ
        } else {
            ungetc(next_c, file); token.type = T_ID;
        }
    }
    else if (c == '<') {
        int next_c = fgetc(file);
        if (next_c == '=') {
            token.value[1] = '='; token.value[2] = '\0'; token.type = 32; // LEQ
        } else {
            ungetc(next_c, file); token.type = 18; // LT
        }
    }
    else if (c == '>') {
        int next_c = fgetc(file);
        if (next_c == '=') {
            token.value[1] = '='; token.value[2] = '\0'; token.type = 33; // GEQ
        } else {
            ungetc(next_c, file); token.type = 24; // GT
        }
    }
    else if (c == '(') token.type = 15;
    else if (c == ')') token.type = 16;
    else if (c == ';') token.type = 21; 
    else if (c == '+') token.type = 19;
    else if (c == '-') token.type = 56; 
    else if (c == '{') token.type = 22;
    else if (c == '}') token.type = 23;
    else token.type = T_ID;

    return token;
}
