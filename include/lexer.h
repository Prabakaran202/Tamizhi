#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

typedef enum {
    T_INT, T_STR, T_FLT, T_BOOL,
    T_IF, T_ELSE, T_FOR, T_WHILE,
    T_RET, T_FUNC, T_PRINT,
    T_INP, T_IMP, T_ID, T_NUM, T_EOF,T_MAIN,T_CALL
} T_Type;

typedef struct {
    T_Type type;
    char value[64];
} Token;

Token get_next_token(FILE *file);
T_Type get_keyword_type(char* value);

#endif
