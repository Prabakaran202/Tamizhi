#include "codegen_bridge.h"

void tamizhi_gen_loop_start(int limit) {
    if (current_function == NULL) {
        current_function = LLVMGetNamedFunction(module, "main");
    }

    LLVMBasicBlockRef pre_loop_bb = LLVMGetInsertBlock(builder);

    loop_top++;
    LoopContext* ctx = &loop_stack[loop_top];

    char cond_name[32], body_name[32], after_name[32], i_name[32];
    snprintf(cond_name, sizeof(cond_name), "loop_cond_%d", loop_counter);
    snprintf(body_name, sizeof(body_name), "loop_body_%d", loop_counter);
    snprintf(after_name, sizeof(after_name), "loop_after_%d", loop_counter);
    snprintf(i_name, sizeof(i_name), "__loop_i_%d", loop_counter);
    loop_counter++;

    ctx->cond_block = LLVMAppendBasicBlockInContext(context, current_function, cond_name);
    ctx->body_block = LLVMAppendBasicBlockInContext(context, current_function, body_name);
    ctx->after_block = LLVMAppendBasicBlockInContext(context, current_function, after_name);

    ctx->i_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), i_name);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0), ctx->i_ptr);

    if (pre_loop_bb && LLVMGetBasicBlockTerminator(pre_loop_bb) == NULL) {
        LLVMBuildBr(builder, ctx->cond_block);
    }

    LLVMPositionBuilderAtEnd(builder, ctx->cond_block);
    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), ctx->i_ptr, "i_val");
    LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntSLT, i_val, LLVMConstInt(LLVMInt32TypeInContext(context), limit, 0), "loop_cond");
    LLVMBuildCondBr(builder, cond, ctx->body_block, ctx->after_block);

    LLVMPositionBuilderAtEnd(builder, ctx->body_block);
}
