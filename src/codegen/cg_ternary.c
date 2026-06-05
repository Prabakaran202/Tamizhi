#include "codegen_bridge.h"

void tamizhi_gen_ternary(char* res_name, char* v1, char* op, char* var2, char* true_val, char* false_val) {
    char clean_res[100], clean_v1[100], clean_v2[100], clean_t[100], clean_f[100];
    snprintf(clean_res, sizeof(clean_res), "%s", res_name); tamizhi_codegen_trim(clean_res);
    snprintf(clean_v1, sizeof(clean_v1), "%s", v1); tamizhi_codegen_trim(clean_v1);
    snprintf(clean_v2, sizeof(clean_v2), "%s", var2); tamizhi_codegen_trim(clean_v2);
    snprintf(clean_t, sizeof(clean_t), "%s", true_val); tamizhi_codegen_trim(clean_t);
    snprintf(clean_f, sizeof(clean_f), "%s", false_val); tamizhi_codegen_trim(clean_f);

    LLVMValueRef val1 = NULL, val2 = NULL;

    if (isdigit((unsigned char)clean_v1[0]) || clean_v1[0] == '-') {
        val1 = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_v1), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_v1) == 0) {
                val1 = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "t_v1");
                break;
            }
        }
    }

    if (isdigit((unsigned char)clean_v2[0]) || clean_v2[0] == '-') {
        val2 = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_v2), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_v2) == 0) {
                val2 = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "t_v2");
                break;
            }
        }
    }

    if (!val1) val1 = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    if (!val2) val2 = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);

    LLVMIntPredicate pred = LLVMIntEQ;
    if (strcmp(op, "<") == 0) pred = LLVMIntSLT;
    else if (strcmp(op, ">") == 0) pred = LLVMIntSGT;
    else if (strcmp(op, "==") == 0) pred = LLVMIntEQ;
    else if (strcmp(op, "!=") == 0) pred = LLVMIntNE;
    else if (strcmp(op, "<=") == 0) pred = LLVMIntSLE;
    else if (strcmp(op, ">=") == 0) pred = LLVMIntSGE;

    LLVMValueRef cond = LLVMBuildICmp(builder, pred, val1, val2, "ternary_cond");

    LLVMValueRef t_val = NULL, f_val = NULL;

    if (isdigit((unsigned char)clean_t[0]) || clean_t[0] == '-') {
        t_val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_t), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_t) == 0) {
                t_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "t_val");
                break;
            }
        }
    }

    char *semi_p = strchr(clean_f, ';');
    if (semi_p != NULL) *semi_p = '\0';
    tamizhi_codegen_trim(clean_f);

    if (isdigit((unsigned char)clean_f[0]) || clean_f[0] == '-') {
        f_val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_f), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_f) == 0) {
                f_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "f_val");
                break;
            }
        }
    }

    if (!t_val) t_val = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    if (!f_val) f_val = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);

    LLVMValueRef select_res = LLVMBuildSelect(builder, cond, t_val, f_val, "ternary_sel");

    LLVMValueRef target_ptr = NULL;
    int target_idx = -1;
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, clean_res) == 0) {
            target_ptr = symbol_table[i].alloca_ptr;
            target_idx = i;
            break;
        }
    }
    if (!target_ptr && var_count < MAX_VARS) {
        if (current_function == NULL) {
            current_function = LLVMGetNamedFunction(module, "main");
        }
        target_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), clean_res);
        snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
        symbol_table[var_count].alloca_ptr = target_ptr;
        symbol_table[var_count].is_str_type = 0;
        target_idx = var_count;
        var_count++;
    }
    if (target_ptr && target_idx != -1) {
        LLVMBuildStore(builder, select_res, target_ptr);
        symbol_table[target_idx].has_static_val = 0; 
    }
}
