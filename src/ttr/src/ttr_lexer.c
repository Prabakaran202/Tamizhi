#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../core/ttr_lexer.h"

static const char *source;
static int pos = 0;

void ttr_init_lexer(const char *source_code) { source = source_code; pos = 0; }

Token get_next_token() {
    Token token;
    token.value[0] = '\0';

    while (source[pos] != '\0' && isspace(source[pos])) pos++;

    if (source[pos] == '\0') { token.type = TOKEN_EOF; return token; }

    // '=' குறியீட்டைக் கண்டுபிடிக்க
    if (source[pos] == '=') {
        token.type = TOKEN_ASSIGN;
        strcpy(token.value, "=");
        pos++;
        return token;
    }

    if (source[pos] == '(') { token.type = TOKEN_LPAREN; strcpy(token.value, "("); pos++; return token; }
    if (source[pos] == ')') { token.type = TOKEN_RPAREN; strcpy(token.value, ")"); pos++; return token; }

    if (source[pos] == '"' || source[pos] == '\'') {
        char quote = source[pos]; pos++;
        int i = 0;
        while (source[pos] != '\0' && source[pos] != quote) token.value[i++] = source[pos++];
        token.value[i] = '\0';
        if (source[pos] == quote) pos++;
        token.type = TOKEN_STRING;
        return token;
    }

    // எண்களைக் கண்டுபிடிக்க (Numbers)
    if (isdigit(source[pos])) {
        int i = 0;
        while (isdigit(source[pos])) token.value[i++] = source[pos++];
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }

    // வார்த்தைகள் மற்றும் Variables (Identifiers)
    if (isalpha(source[pos])) {
        int i = 0;
        while (isalnum(source[pos]) || source[pos] == '_') token.value[i++] = source[pos++];
        token.value[i] = '\0';

        if (strcmp(token.value, "print") == 0) token.type = TOKEN_PRINT;
        else token.type = TOKEN_IDENTIFIER; // Variables: name, age etc.
        
        return token;
    }

    pos++;
    token.type = TOKEN_EOF;
    return token;
}
