#include "../include/codegen_bridge.h"
#include <string.h>

void tamizhi_gen_while_start(char* var1, char* op, char* var2) {
    if (current_function == NULL) {
        current_function = LLVMGetNamedFunction(module, "main");
    }

    LLVMBasicBlockRef pre_loop_bb = LLVMGetInsertBlock(builder);

    // 🌟 1. Stack-ல் புதிய லூப்பிற்கான இடத்தை உருவாக்குதல்
    loop_top++;
    LoopContext* ctx = &loop_stack[loop_top];

    // 🌟 2. Block-களுக்குத் தனித்துவமான பெயர்களை உருவாக்குதல்
    char cond_name[32], body_name[32], after_name[32];
    snprintf(cond_name, sizeof(cond_name), "while_cond_%d", loop_counter);
    snprintf(body_name, sizeof(body_name), "while_body_%d", loop_counter);
    snprintf(after_name, sizeof(after_name), "while_after_%d", loop_counter);
    loop_counter++;

    // 🌟 3. Block-களை உருவாக்கி Stack-ல் சேமித்தல்
    ctx->cond_block = LLVMAppendBasicBlockInContext(context, current_function, cond_name);
    ctx->body_block = LLVMAppendBasicBlockInContext(context, current_function, body_name);
    ctx->after_block = LLVMAppendBasicBlockInContext(context, current_function, after_name);

    if (pre_loop_bb && LLVMGetBasicBlockTerminator(pre_loop_bb) == NULL) {
        LLVMBuildBr(builder, ctx->cond_block);
    }

    // 🌟 4. Condition பிளாக்கிற்குள் செல்லுதல்
    LLVMPositionBuilderAtEnd(builder, ctx->cond_block);

    LLVMValueRef condition_val;
    if (strcmp(var1, "1") == 0) {
        // 🌐 Infinite Loop (for (1)...)
        condition_val = LLVMConstInt(LLVMInt1TypeInContext(context), 1, 0); 
    } else {
        // 🔄 சாதாரண While Loop-க்கான கண்டிஷன் (தற்போதைக்கு True)
        condition_val = LLVMConstInt(LLVMInt1TypeInContext(context), 1, 0);
    }

    // 🌟 5. Condition-ஐச் சரிபார்த்து பிரித்து அனுப்புதல்
    LLVMBuildCondBr(builder, condition_val, ctx->body_block, ctx->after_block);

    // 🌟 6. Body Block-க்குள் நுழைதல்
    LLVMPositionBuilderAtEnd(builder, ctx->body_block);
}
