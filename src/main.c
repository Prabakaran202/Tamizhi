#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 1. Enums - டோக்கன் வகைகள்
typedef enum {
    T_INT, T_STR, T_FLT, T_BOOL,      // எ, வ, பு, உ
    T_IF, T_ELSE, T_FOR, T_WHILE,     // ஆ2, இ, சு, சு2
    T_RET, T_FUNC, T_PRINT,           // த, செயல், கூறு
    T_INP, T_IMP, T_ID, T_NUM, T_EOF 
} T_Type;

// 2. Token Structure - டோக்கன் அமைப்பு
typedef struct {
    T_Type type;
    char value[64];
} Token;

// 3. Keyword Helper - தமிழ் வார்த்தைகளை அடையாளம் காண
T_Type get_keyword_type(char* value) {
    if (strcmp(value, "செயல்") == 0) return T_FUNC;
    if (strcmp(value, "கூறு") == 0) return T_PRINT;
    if (strcmp(value, "கேள்") == 0) return T_INP;
    if (strcmp(value, "சேர்") == 0) return T_IMP;
    if (strcmp(value, "எ") == 0) return T_INT;
    if (strcmp(value, "வ") == 0) return T_STR;
    if (strcmp(value, "ஆ2") == 0) return T_IF;
    if (strcmp(value, "சு") == 0) return T_FOR;
    if (strcmp(value, "சு2") == 0) return T_WHILE;
    if (strcmp(value, "த") == 0) return T_RET;
    return T_ID; 
}

// 4. Lexer Function - கோப்பை டோக்கன்களாகப் பிரிக்க
Token get_next_token(FILE *file) {
    Token token;
    int c = fgetc(file);

    while (isspace(c)) c = fgetc(file);

    if (c == EOF) {
        token.type = T_EOF;
        strcpy(token.value, "EOF");
        return token;
    }

    // தமிழ் மற்றும் அடையாளங்கள்
    if (isalpha(c) || c > 127) {
        int i = 0;
        do {
            token.value[i++] = c;
            c = fgetc(file);
        } while (isalnum(c) || c > 127 || c == '2'); 
        ungetc(c, file);
        token.value[i] = '\0';
        token.type = get_keyword_type(token.value);
        return token;
    }

    // எண்கள்
    if (isdigit(c)) {
        int i = 0;
        while (isdigit(c)) {
            token.value[i++] = c;
            c = fgetc(file);
        }
        ungetc(c, file);
        token.value[i] = '\0';
        token.type = T_NUM;
        return token;
    }

    // சிம்பல்ஸ்
    token.value[0] = c;
    token.value[1] = '\0';
    token.type = T_ID; // தற்காலிகமாக

    if (strchr("=(){};+-*/<>", c)) return token;

    return token;
}

// 5. Main Function
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("பயன்பாடு: tamizhi <filename.tz>\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("கோப்பை திறக்க முடியவில்லை");
        return 1;
    }

    Token t;
    printf("--- தமிழி கம்பைலர் (v0.1) ---\n");
    
    while ((t = get_next_token(file)).type != T_EOF) {
        printf("வகை: %d | மதிப்பு: %s\n", t.type, t.value);
    }

    fclose(file);
    printf("\nஆய்வு முடிந்தது.\n");
    return 0;
}
