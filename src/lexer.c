#include "lexer.h"
#include <string.h>
#include <ctype.h>

T_Type get_keyword_type(char* value) {
    if (strcmp(value, "முதன்மை") ==0) return T_MAIN;
    if (strcmp(value, "நிகழ்") == 0) return T_FUNC;
    if (strcmp(value, "அச்சிடு") == 0) return T_PRINT;
    if (strcmp(value, "உள்ளீடு") == 0) return T_INP;
    if (strcmp(value, "சேர்") == 0) return T_IMP;
    if (strcmp(value, "முழுஎண்") == 0) return T_INT;
    if (strcmp(value, "மாலை") == 0) return T_STR;
    if (strcmp(value, "என்றால்") == 0) return T_IF;
    if (strcmp(value, "சு") == 0) return T_FOR;
    if (strcmp(value, "சு2") == 0) return T_WHILE;
    if (strcmp(value, "திரும்பக்கொடு") == 0) return T_RET;
    if (strcmp(value, "மெய்பொய்") == 0) return T_BOOL;
    if (strcmp(value, "இயக்கு") == 0) return T_CALL;
    return T_ID; 
}

Token get_next_token(FILE *file) {
    Token token;
    int c = fgetc(file);

    while (isspace(c)) c = fgetc(file);

    if (c == EOF) {
        token.type = T_EOF;
        strcpy(token.value, "EOF");
        return token;
    }

    if (isalpha(c) || (unsigned char)c > 127) {
        int i = 0;
        do {
            token.value[i++] = c;
            c = fgetc(file);
        } while (isalnum(c) || (unsigned char)c > 127 || c == '2'); 
        ungetc(c, file);
        token.value[i] = '\0';
        token.type = get_keyword_type(token.value);
        return token;
    }

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

    token.value[0] = c;
    token.value[1] = '\0';
    token.type = T_ID; 

    return token;
}
