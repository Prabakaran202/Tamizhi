#include "codegen_bridge.h"

void tamizhi_gen_else_start(void) {
    if (if_top < 0) return;
    if_stack[if_top].has_else = 1;
    LLVMPositionBuilderAtEnd(builder, if_stack[if_top].false_block);
}
