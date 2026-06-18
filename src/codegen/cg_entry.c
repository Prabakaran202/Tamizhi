#include "codegen_bridge.h"

void tamizhi_generate_entry(void) {
    if (LLVMGetNamedFunction(module, "main")) return; 

    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32TypeInContext(context), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);

    current_function = main_func; 

    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(context, main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry); 
}
