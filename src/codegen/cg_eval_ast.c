#include "codegen_bridge.h"

LLVMValueRef tamizhi_evaluate_ast(ASTNode* node) {
    if (node == NULL) return NULL;

    if (node->type == AST_NUMBER) {
        return LLVMConstInt(LLVMInt32TypeInContext(context), node->data.num_value, 0);
    }
    else if (node->type == AST_IDENTIFIER) {
        char clean_name[256];
        snprintf(clean_name, sizeof(clean_name), "%s", node->data.var_name);
        tamizhi_codegen_trim(clean_name);

        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_name) == 0) {
                return LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "ast_load");
            }
        }
        return LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0); 
    }
    else if (node->type == AST_BINARY_OP) {
        LLVMValueRef left_val = tamizhi_evaluate_ast(node->data.binop.left);
        LLVMValueRef right_val = tamizhi_evaluate_ast(node->data.binop.right);

        if (!left_val || !right_val) return NULL;

        if (strcmp(node->data.binop.op, "+") == 0) {
            return LLVMBuildAdd(builder, left_val, right_val, "ast_add");
        } else if (strcmp(node->data.binop.op, "-") == 0) {
            return LLVMBuildSub(builder, left_val, right_val, "ast_sub");
        } else if (strcmp(node->data.binop.op, "*") == 0) {
            return LLVMBuildMul(builder, left_val, right_val, "ast_mul");
        } else if (strcmp(node->data.binop.op, "/") == 0) {
            return LLVMBuildSDiv(builder, left_val, right_val, "ast_div");
        }
    }
    return NULL;
}
