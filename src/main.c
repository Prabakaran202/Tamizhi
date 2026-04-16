#include <stdio.h>
#include "lexer.h"
#include "codegen.h"

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

    printf("--- தமிழி கம்பைலர் (v0.1) ---\n");

    // 1. LLVM Engine-a initialize pannuvom
    tamizhi_codegen_init();

    // 2. Entry point (main function) create pannuvom
    tamizhi_generate_entry();

    // 3. Tokens-a analyze panni screen-la print pannuvom (Temporary)
    Token t;
    while ((t = get_next_token(file)).type != T_EOF) {
        printf("வகை: %d | மதிப்பு: %s\n", t.type, t.value);
    }

    // 4. Mudivila LLVM IR-a print panni finish pannuvom
    tamizhi_codegen_finish();

    fclose(file);
    printf("\nஆய்வு முடிந்தது.\n");
    return 0;
}
