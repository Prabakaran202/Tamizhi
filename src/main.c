#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // Itha extra-va add pannunga (isspace-ku thevai)

// 1. Enums (Already iruku)
typedef enum { ... } T_Type;

// 2. Token Struct (Puthusa paste pannunga)
typedef struct {
    T_Type type;
    char value[64];
} Token;

// 3. Keyword Helper (Puthusa paste pannunga)
T_Type get_keyword_type(char* value) {
    if (strcmp(value, "செயல்") == 0) return T_FUNC;
    if (strcmp(value, "கூறு") == 0) return T_PRINT;
    if (strcmp(value, "கேள்") == 0) return T_INP;
    if (strcmp(value, "சேர்") == 0) return T_IMP;
    if (strcmp(value, "எ") == 0) return T_INT;
    if (strcmp(value, "ஆ2") == 0) return T_IF;
    if (strcmp(value, "த") == 0) return T_RET;
    return T_ID; 
}

// 4. Advanced Lexer Function (Puthusa paste pannunga)
Token get_next_token(FILE *file) {
    Token token;
    int c = fgetc(file);
    while (isspace(c)) c = fgetc(file); // Spaces skip panna

    if (c == EOF) {
        token.type = T_EOF;
        return token;
    }

    // Unicode & Identifier handling
    if (isalpha(c) || c > 127) {
        int i = 0;
        do {
            token.value[i++] = c;
            c = fgetc(file);
        } while (isalnum(c) || c > 127);
        ungetc(c, file);
        token.value[i] = '\0';
        token.type = get_keyword_type(token.value);
        return token;
    }

    // Symbols handling (Ithaiyum add pannunga)
    if (c == '=') {
        token.type = T_ID; // Temporary-ah assign panrom
        strcpy(token.value, "=");
        return token;
    }

    return token;
}

// 5. Main Function (Itha update pannunga)
int main(int argc, char *argv[]) {
    // ... file handling logic ...
    Token t;
    while ((t = get_next_token(file)).type != T_EOF) {
        printf("வகை: %d | மதிப்பு: %s\n", t.type, t.value);
    }
    // ... close logic ...
}
