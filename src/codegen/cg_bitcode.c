#include "codegen_bridge.h"

void tamizhi_generate_universal_bitcode(const char* filename) {
    if (LLVMWriteBitcodeToFile(module, filename) != 0) {
        fprintf(stderr, " [Error] Failed to write universal bitcode!\n");
    } else {
        if (tamizhi_debug_mode){
            fprintf(stderr, " [Universal] Bitcode generated: %s\n", filename);
        }
    }
    
    // 🌟 THE MASTER FIX: ஆரம்பத்திலேயே பாதுகாப்பாக Initialize செய்துவிடுகிறோம்!
    char asm_path[256] = "storage/output.ll";
    
    // Debug Mode-ல் இருந்தால் மட்டுமே .ll ஃபைலை டிஸ்க்கில் எழுதுகிறோம்
    if (tamizhi_debug_mode) {
        FILE *f = fopen(asm_path, "w");
        if (f) {
            char *str = LLVMPrintModuleToString(module);
            fprintf(f, "%s", str);
            LLVMDisposeMessage(str);
            fclose(f);
        }
    }

