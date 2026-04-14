#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tamizhi Keywords-ah identify panna enum
typedef enum {
    T_INT, T_STR, T_FLT, T_BOOL,      // எ, வ, பு, உ
    T_IF, T_ELSE, T_FOR, T_WHILE,     // ஆ2, இ, சு, சு2
    T_RET, T_FUNC, T_PRINT,           // த, செயல், கூறு
    T_INP, T_IMP, T_ID, T_NUM, T_EOF  // கேள், சேர், identifiers
} T_Type;

void analyze_token(char* word) {
    // Action Keywords (Full Words)
    if (strcmp(word, "செயல்") == 0) printf("TOKEN: FUNCTION (செயல்)\n");
    else if (strcmp(word, "கூறு") == 0) printf("TOKEN: PRINT (கூறு)\n");
    else if (strcmp(word, "கேள்") == 0) printf("TOKEN: INPUT (கேள்)\n");
    else if (strcmp(word, "சேர்") == 0) printf("TOKEN: IMPORT (சேர்)\n");
    
    // Data Types & Logic (Single/Special Letters)
    // Note: Unicode handling requires strncmp for multi-byte characters
    else if (strncmp(word, "எ", 3) == 0) printf("TOKEN: INT (எ)\n");
    else if (strncmp(word, "ஆ2", 4) == 0) printf("TOKEN: IF (ஆ2)\n");
    else if (strncmp(word, "த", 3) == 0) printf("TOKEN: RETURN (த)\n");
    else printf("TOKEN: IDENTIFIER (%s)\n", word);
}

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

    char buffer[255];
    printf("--- தமிழி கம்பைலர் (v0.1) ---\n");
    
    // Simple Lexer: Spacing vachu words-ah pirikuthu
    while (fscanf(file, "%s", buffer) != EOF) {
        analyze_token(buffer);
    }

    fclose(file);
    printf("\nஆய்வு முடிந்தது.\n");
    return 0;
}
