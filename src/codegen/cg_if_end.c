#include "codegen_bridge.h"

void tamizhi_gen_if_end(void) {
    if (if_top < 0) return;

    IfContext* ctx = &if_stack[if_top];
    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);

    if (current_bb && LLVMGetBasicBlockTerminator(current_bb) == NULL) {
        LLVMBuildBr(builder, ctx->end_block);
    }

    if (!ctx->has_else) {
        LLVMPositionBuilderAtEnd(ctx->false_block);
        if (LLVMGetBasicBlockTerminator(ctx->false_block) == NULL) {
            LLVMBuildBr(builder, ctx->end_block);
        }
    }

    LLVMPositionBuilderAtEnd(builder, ctx->end_block);
    if_top--;
}
