#include "codegen_bridge.h"

void tamizhi_gen_function_start(char* func_name) {
    char clean_name[100];
    snprintf(clean_name, sizeof(clean_name), "%s", func_name);
    tamizhi_codegen_trim(clean_name);

    if (LLVMGetNamedFunction(module, clean_name)) return;

    LLVMTypeRef func_type = LLVMFunctionType(LLVMInt32TypeInContext(context), NULL, 0, 0);
    LLVMValueRef function = LLVMAddFunction(module, clean_name, func_type);

    current_function = function; 

    LLVMBasicBlockRef entry_block = LLVMAppendBasicBlockInContext(context, function, "entry");
    LLVMPositionBuilderAtEnd(builder, entry_block);

    if (func_count < MAX_FUNCS) {
        snprintf(function_table[func_count].name, sizeof(function_table[func_count].name), "%s", clean_name);
        function_table[func_count].func_ref = function;
        function_table[func_count].resume_block = entry_block;
        func_count++;
    }
}
