#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h" // Intha header romba mukkiyam
#include "codegen.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr,"பயன்பாடு: tamizhi <filename.tz>\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("கோப்பை திறக்க முடியவில்லை");
        return 1;
    }

    // 1. Backend Engine Setup
    tamizhi_codegen_init();
    tamizhi_generate_entry();

    fprintf(stderr,"--- தமிழி கம்பைலர் (v0.1) ---\n");

    // 2. Parser Integration 🔥
    // Intha 'parse' function kulla thaan Lexer and Codegen onnaa serum.
    // Lexer tokens-a tharum, Parser athai analyze panni Codegen-a call pannum.
    parse(file); 

    // 3. Finalize and Output IR
    tamizhi_codegen_finish();

    fclose(file);
    fprintf(stderr,"\nதொகுப்பு மற்றும் ஆய்வு முடிந்தது.\n");
    return 0;
}
