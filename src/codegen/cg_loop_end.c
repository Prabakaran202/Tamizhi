#include "codegen_bridge.h"

void tamizhi_gen_loop_end(void) {
    if(loop_top < 0) return;
    LoopContext* ctx = &loop_stack[loop_top];

    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);

    if (current_bb && LLVMGetBasicBlockTerminator(current_bb) == NULL) {
        LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), ctx->i_ptr, "i_val");
        LLVMValueRef next_val = LLVMBuildAdd(builder, i_val, LLVMConstInt(LLVMInt32TypeInContext(context), 1, 0), "next_i");
        LLVMBuildStore(builder, next_val, ctx->i_ptr);
        LLVMBuildBr(builder, ctx->cond_block); 
    }

    LLVMPositionBuilderAtEnd(builder, ctx->after_block);
    loop_top--;
}
