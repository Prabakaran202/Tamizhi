#include "codegen.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <stdio.h>

// LLVM Global variables
LLVMModuleRef module;
LLVMBuilderRef builder;

void tamizhi_codegen_init() {
    // 1. Module create pannuvom (Intha file-oda name)
    module = LLVMModuleCreateWithName("tamizhi_engine");

    // 2. Builder create pannuvom (Instructions ezhutha ithu thaan venum)
    builder = LLVMCreateBuilder();

    printf("[Codegen] LLVM Engine initialized successfully on your phone!\n");
}

void tamizhi_generate_entry() {
    // 3. 'main' function-a create pannanum
    LLVMTypeRef return_type = LLVMInt32Type();
    LLVMTypeRef main_func_type = LLVMFunctionType(return_type, NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);

    // 4. Function kulla enter aaguvom (Entry block)
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);

    printf("[Codegen] Main function entry point created.\n");
}

void tamizhi_codegen_finish() {
    // 5. Code-a verify panni print pannanum
    char *error = NULL;
    LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
    
    // Inga thaan unga code IR-a (Intermediate Representation) screen-la kaatum
    LLVMDumpModule(module);

    // Clean up
    LLVMDisposeBuilder(builder);
}
