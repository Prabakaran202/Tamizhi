#include "codegen_bridge.h"

void tamizhi_gen_function_end(void) {
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) {
        LLVMBuildRet(builder, LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0));
    }

    LLVMValueRef main_fn = LLVMGetNamedFunction(module, "main");
    current_function = main_fn;
    if (main_fn) {
        LLVMBasicBlockRef bb = LLVMGetFirstBasicBlock(main_fn);
        LLVMBasicBlockRef open_bb = NULL;
        while (bb) {
            if (LLVMGetBasicBlockTerminator(bb) == NULL) {
                open_bb = bb;
                break;
            }
            bb = LLVMGetNextBasicBlock(bb);
        }
        if (open_bb) {
            LLVMPositionBuilderAtEnd(builder, open_bb);
        } else {
            LLVMBasicBlockRef cont = LLVMAppendBasicBlockInContext(context, main_fn, "main_cont");
            LLVMPositionBuilderAtEnd(builder, cont);
        }
    }
}
