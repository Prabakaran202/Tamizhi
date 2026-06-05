#include "codegen_bridge.h"

void tamizhi_gen_if_body_end(void) {
    if (if_top < 0) return;
    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
    if (current_bb && LLVMGetBasicBlockTerminator(current_bb) == NULL) {
        LLVMBuildBr(builder, if_stack[if_top].end_block);
    }
}
