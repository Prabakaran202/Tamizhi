#include "codegen_bridge.h"

void tamizhi_gen_if_start(char* lhs, char* rel_op, char* rhs) {
    char clean_lhs[100], clean_rhs[100], clean_op[10];
    snprintf(clean_lhs, sizeof(clean_lhs), "%s", lhs); tamizhi_codegen_trim(clean_lhs);
    snprintf(clean_rhs, sizeof(clean_rhs), "%s", rhs); tamizhi_codegen_trim(clean_rhs);
    snprintf(clean_op, sizeof(clean_op), "%s", rel_op); tamizhi_codegen_trim(clean_op);

    if_top++;
    if (if_top >= MAX_IF_DEPTH - 1) return;

    if (current_function == NULL) {
        current_function = LLVMGetNamedFunction(module, "main");
    }

    LLVMValueRef val1 = NULL, val2 = NULL;

    if (isdigit((unsigned char)clean_lhs[0]) || clean_lhs[0] == '-') {
        val1 = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_lhs), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_lhs) == 0) {
                val1 = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "if_v1");
                break;
            }
        }
    }

    if (isdigit((unsigned char)clean_rhs[0]) || clean_rhs[0] == '-') {
        val2 = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_rhs), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_rhs) == 0) {
                val2 = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "if_v2");
                break;
            }
        }
    }

    if (!val1) val1 = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    if (!val2) val2 = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);

    LLVMIntPredicate pred = LLVMIntEQ;
    if (strcmp(clean_op, "<") == 0) pred = LLVMIntSLT;
    else if (strcmp(clean_op, ">") == 0) pred = LLVMIntSGT;
    else if (strcmp(clean_op, "==") == 0) pred = LLVMIntEQ;
    else if (strcmp(clean_op, "!=") == 0) pred = LLVMIntNE;
    else if (strcmp(clean_op, "<=") == 0) pred = LLVMIntSLE;
    else if (strcmp(clean_op, ">=") == 0) pred = LLVMIntSGE;

    LLVMValueRef cond = LLVMBuildICmp(builder, pred, val1, val2, "if_cond");

    char true_name[32], false_name[32], end_name[32];
    snprintf(true_name, sizeof(true_name), "if_true_%d", if_counter);
    snprintf(false_name, sizeof(false_name), "if_false_%d", if_counter);
    snprintf(end_name, sizeof(end_name), "if_end_%d", if_counter);
    if_counter++;

    if_stack[if_top].true_block = LLVMAppendBasicBlockInContext(context, current_function, true_name);
    if_stack[if_top].false_block = LLVMAppendBasicBlockInContext(context, current_function, false_name);
    if_stack[if_top].end_block = LLVMAppendBasicBlockInContext(context, current_function, end_name);
    if_stack[if_top].has_else = 0;

    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
    if (current_bb && LLVMGetBasicBlockTerminator(current_bb) == NULL) {
        LLVMBuildCondBr(builder, cond, if_stack[if_top].true_block, if_stack[if_top].false_block);
    }

    LLVMPositionBuilderAtEnd(builder, if_stack[if_top].true_block);
}
