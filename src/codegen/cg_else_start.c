#include "codegen_bridge.h"

void tamizhi_gen_else_start(void) {
    if (if_top < 0) return;
    
    // 1. Else இருக்கிறது என உறுதிப்படுத்துகிறோம்
    if_stack[if_top].has_else = 1;

    // 2. CRITICAL FIX: true_block-ன் இறுதியில் டெர்மினேட்டர் (Branch to end_block) இருப்பதை உறுதிசெய்கிறோம்
    LLVMBasicBlockRef true_bb = if_stack[if_top].true_block;
    
    // தற்காலிகமாக பில்டரை true_bb-ன் முடிவுக்குக் கொண்டு செல்கிறோம்
    LLVMPositionBuilderAtEnd(builder, true_bb);
    
    if (LLVMGetBasicBlockTerminator(true_bb) == NULL) {
        LLVMBuildBr(builder, if_stack[if_top].end_block);
    }

    // 3. இப்போது பாதுகாப்பாக பில்டரை false_block-க்கு நகர்த்துகிறோம் (Else Body-ஐ எழுத)
    LLVMPositionBuilderAtEnd(builder, if_stack[if_top].false_block);
}
