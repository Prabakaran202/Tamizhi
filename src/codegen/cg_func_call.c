#include "codegen_bridge.h"

void tamizhi_gen_function_call(char* func_name) {
    char clean_name[100];
    snprintf(clean_name, sizeof(clean_name), "%s", func_name);
    tamizhi_codegen_trim(clean_name);

    LLVMValueRef target_func = LLVMGetNamedFunction(module, clean_name);
    if (target_func) {
        LLVMTypeRef func_type = LLVMFunctionType(LLVMInt32TypeInContext(context), NULL, 0, 0);
        LLVMBuildCall2(builder, func_type, target_func, NULL, 0, "call_tmp");
    }
}
