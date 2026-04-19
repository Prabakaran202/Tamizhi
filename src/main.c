/*#include <stdio.h>
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
    // codegen.c kulla
 void tamizhi_codegen_finish() {
    LLVMDumpModule(module); // Ithu terminal-la mattum kaattum
    // Namma file-ku anuppa ithu venum:
    char *ir = LLVMPrintModuleToString(module);
    printf("%s", ir); 
    LLVMDisposeMessage(ir);
}

    tamizhi_codegen_finish();

    fclose(file);
    fprintf(stderr,"\nதொகுப்பு மற்றும் ஆய்வு முடிந்தது.\n");
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

// 1. Function Definition (Veliya irukanum)
void tamizhi_codegen_finish() {
    // Note: 'module' variable 'codegen.h'-la extern-ah irukanum
    LLVMDumpModule(module); 
    char *ir = LLVMPrintModuleToString(module);
    printf("%s", ir); 
    LLVMDisposeMessage(ir);
}

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

    tamizhi_codegen_init();
    tamizhi_generate_entry();

    fprintf(stderr,"--- தமிழி கம்பைலர் (v0.1) ---\n");

    parse(file); 

    // 2. Function Call (Inga thaan call pannanum)
    tamizhi_codegen_finish();

    fclose(file);
    fprintf(stderr,"\nதொகுப்பு மற்றும் ஆய்வு முடிந்தது.\n");
    return 0;
}

*/
#include <stdio.h>
#include <stdlib.h>
#include <llvm-c/Core.h> // LLVM types-ku ithu venum
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

// 1. Extern reference: 'module' variable vera file-la (codegen.c) irukku nu compiler-ku solrom
extern LLVMModuleRef module; 

// 2. Function Definition (Clean and Outside main)
void tamizhi_codegen_finish() {
    if (module == NULL) {
        fprintf(stderr, "Error: LLVM Module initialization fail aayiduchi!\n");
        return;
    }
    
    // Terminal-la debug panna dump pannuvom
    LLVMDumpModule(module); 
    
    // File-kulla (stdout) IR code-ah anuppa ithu mukkiyam
    char *ir = LLVMPrintModuleToString(module);
    if (ir) {
        fprintf(stderr, "%s", ir); 
        LLVMDisposeMessage(ir);
    }
}

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

    // Backend setup
    tamizhi_codegen_init();
    tamizhi_generate_entry();

    fprintf(stderr,"--- தமிழி கம்பைலர் (v0.1) ---\n");

    // Lexing and Parsing starts
    parse(file); 

    // Finalize output
    tamizhi_codegen_finish();

    fclose(file);
    fprintf(stderr,"\nதொகுப்பு மற்றும் ஆய்வு முடிந்தது.\n");
    return 0;
}


