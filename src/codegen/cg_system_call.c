#include "codegen_bridge.h"

// =========================================================================
// 🚀 தமிழி லினக்ஸ் சிஸ்டம் கால் மேப்பர் (இயக்கு கமாண்ட் கண்ட்ரோலர்)
// =========================================================================
void tamizhi_gen_system_call(char* command) {
    char clean_cmd[1024];
    snprintf(clean_cmd, sizeof(clean_cmd), "%s", command);
    tamizhi_codegen_trim(clean_cmd);

    int len = strlen(clean_cmd);
    // டோக்கன் கொட்டேஷனுடன் வந்தால் (" कமாண்ட் ") அதை மட்டும் பிரித்தெடுக்க பாதுகாப்பு லேயர்
    if (clean_cmd[0] == '\"' && clean_cmd[len - 1] == '\"' && len >= 2) {
        char temp[1024];
        memset(temp, 0, sizeof(temp));
        strncpy(temp, clean_cmd + 1, len - 2);
        strcpy(clean_cmd, temp);
    }

    // 1. கமாண்ட் டெக்ஸ்ட்டை LLVM Global String Pointer-ஆக மாற்றுதல்
    LLVMValueRef cmd_ptr = LLVMBuildGlobalStringPtr(builder, clean_cmd, "sys_cmd_lit");

    // 2. libc system() ஃபங்ஷனுக்கான ஆர்கியூமென்ட் தயார் செய்தல்
    LLVMValueRef args[] = { cmd_ptr };

    // 3. LLVM IR லெவலில் Native system(cmd_ptr) இன்ஜெக்ஷன் செய்தல்
    LLVMBuildCall2(builder, system_type, system_func, args, 1, "sys_call_tmp");
    
    fprintf(stderr, " [Shell Engine] Injected Native Linux Syscall: %s\n", clean_cmd);
}
