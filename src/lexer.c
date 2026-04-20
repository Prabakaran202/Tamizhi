#include "lexer.h"
#include <string.h>
#include <ctype.h>

T_Type get_keyword_type(char* value) {
    if (strcmp(value, "முதன்மை") == 0 || strcmp(value, "main") == 0) {
    return T_MAIN;}
    if (strcmp(value,"நிகழ்") == 0 || strcmp(value,"fun") = 0){return T_FUN;}
    if (strcmp(value, "அச்சிடு") == 0 || strcmp(value,"print") = 0) {return T_PRINT;}
    if (strcmp(value, "உள்ளீடு") == 0  || strcmp(value,"input") = 0){return T_INP;}
    if (strcmp(value, "சேர்") == 0  || strcmp(value,"import") = 0){return T_IMP;}
    if (strcmp(value, "முழுஎண்") == 0 || strcmp(value, "Num") == 0) {
    return T_INT;
    }

    if (strcmp(value, "Str") == 0) return T_STR;
    if (strcmp(value, "if") == 0) return T_IF;
    if (strcmp(value, "சு") == 0) || strcmp(value,"for") = 0){return T_FOR;}
    if (strcmp(value, "சு2") == 0) || strcmp(value,"while") = 0){return T_WHILE;}
    if (strcmp(value, "return") == 0) return T_RET;
    if (strcmp(value, "bool")== 0) return T_BOOL;
    if (strcmp(value, "இயக்கு") == 0 || strcmp(value,"call") = 0){return T_CALL;}
    return T_ID; 
}
Token get_next_token(FILE *file) {
    Token token;
    int c = fgetc(file);

    // 1. Skip Whitespaces
    while (isspace(c)) c = fgetc(file);

    if (c == EOF) {
        token.type = T_EOF;
        strcpy(token.value, "EOF");
        return token;
    }

    // 2. Handle Tamil Keywords & Identifiers
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

    // 3. Handle Numbers
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

    // 4. CRITICAL: Handle Symbols & Operators
    token.value[0] = c;
    token.value[1] = '\0';

    if (c == '(') token.type = 15;      // T_LPAREN
    else if (c == ')') token.type = 16; // T_RPAREN
    else if (c == ';') token.type = 17; // T_SEMI
    else if (c == '<') token.type = 18; // T_LT
    else if (c == '>') token.type = 21; // T_GT
    else if (c == '+') token.type = 19; // T_PLUS
    else if (c == '=') token.type = 20; // T_ASSIGN
    else if (c == '{') token.type = 22; // T_LBRACE
    else if (c == '}') token.type = 23; // T_RBRACE
    else token.type = T_ID;             // Default to Identifier

    return token;
}
