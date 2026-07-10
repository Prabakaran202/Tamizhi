#include "../include/codegen_bridge.h"

void tamizhi_gen_while_end(void) {
    if(loop_top < 0) return;
    
    // 🌟 1. Stack-ல் இருந்து தற்போதைய லூப்பை எடுத்தல்
    LoopContext* ctx = &loop_stack[loop_top];

    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);

    // 🌟 2. Body-யின் முடிவில் மீண்டும் Condition-க்கே திரும்புதல்
    if (current_bb && LLVMGetBasicBlockTerminator(current_bb) == NULL) {
        LLVMBuildBr(builder, ctx->cond_block); 
    }

    // 🌟 3. லூப் முடிந்து வெளியே வரும் பிளாக்கிற்குச் செல்லுதல்
    LLVMPositionBuilderAtEnd(builder, ctx->after_block);
    
    // 🌟 4. Stack-ல் இருந்து லூப்பை நீக்குதல்
    loop_top--;
}
