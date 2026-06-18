#include "codegen_bridge.h"

LLVMValueRef create_entry_alloca(LLVMValueRef function, LLVMTypeRef type, const char* name) {
    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
    LLVMBasicBlockRef entry_bb = LLVMGetEntryBasicBlock(function);
    LLVMValueRef first_inst = LLVMGetFirstInstruction(entry_bb);

    if (first_inst) {
        LLVMPositionBuilderBefore(builder, first_inst);
    } else {
        LLVMPositionBuilderAtEnd(builder, entry_bb);
    }

    LLVMValueRef alloca = LLVMBuildAlloca(builder, type, name);

    if (current_bb) {
        LLVMPositionBuilderAtEnd(builder, current_bb);
    }

    return alloca;
}
