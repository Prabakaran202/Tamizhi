void tamizhi_gen_system_call(char* command) {
    char clean_cmd[1024];
    snprintf(clean_cmd, sizeof(clean_cmd), "%s", command);
    tamizhi_codegen_trim(clean_cmd);

    int len = strlen(clean_cmd);
    // கோடுல இயக்கு "clear" என்று கொட்டேஷனுடன் கொடுத்தால் அதை மட்டும் பிரித்தெடுக்க
    if (clean_cmd[0] == '\"' && clean_cmd[len - 1] == '\"' && len >= 2) {
        char temp[1024];
        memset(temp, 0, sizeof(temp));
        strncpy(temp, clean_cmd + 1, len - 2);
        strcpy(clean_cmd, temp);
    }

    // 1. கமாண்ட் டெக்ஸ்ட்டை LLVM Global String Pointer-ஆக மாற்றுதல்
    LLVMValueRef cmd_ptr = LLVMBuildGlobalStringPtr(builder, clean_cmd, "sys_cmd_lit");

    // 2. சிஸ்டம் கால் பங்க்ஷனுக்கான ஆர்கியூமென்ட் அரே தயார் செய்தல்
    LLVMValueRef args[] = { cmd_ptr };

    // 3. LLVM IR லெவலில் system(cmd_ptr) கமாண்டை பில்ட் செய்தல்
    LLVMBuildCall2(builder, system_type, system_func, args, 1, "sys_call_tmp");
    
    fprintf(stderr, " [Shell Engine] Injected Native Linux Syscall: %s\n", clean_cmd);
}
