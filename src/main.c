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

    // 1. Backend-a start pannuvom
    tamizhi_codegen_init();

    // 2. Main entry point create pannuvom
    tamizhi_generate_entry();

    // 3. Oru dummy variable create panni LLVM memory-a check pannuvom
    tamizhi_gen_var_decl("i", 0); 

    // 4. Tokens list-a analyze panni screen-la kaatuvom
    Token t;
    while ((t = get_next_token(file)).type != T_EOF) {
        printf("வகை: %d | மதிப்பு: %s\n", t.type, t.value);
    }

    // 5. LLVM IR-a generate panni finalize pannuvom
    tamizhi_codegen_finish();

    fclose(file);
    printf("\nஆய்வு முடிந்தது.\n");
    return 0;
}
