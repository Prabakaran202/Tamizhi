#include "codegen_bridge.h"

void tamizhi_generate_universal_bitcode(const char* filename) {
    if (LLVMWriteBitcodeToFile(module, filename) != 0) {
        fprintf(stderr, " [Error] Failed to write universal bitcode!\n");
    } else {
        if (tamizhi_debug_mode){
        fprintf(stderr, " [Universal] Bitcode generated: %s\n", filename);
    }
    }
    char asm_path[256];
    if (tamizhi_debug_mode){
    sprintf(asm_path, "storage/output.ll");
    }
    FILE *f = fopen(asm_path, "w");
    if (f) {
        char *str = LLVMPrintModuleToString(module);
        fprintf(f, "%s", str);
        LLVMDisposeMessage(str);
        fclose(f);
    }
}

