#include "codegen_bridge.h"

void tamizhi_gen_assign_from_return(char* var_name) {
    char clean_name[100];
    snprintf(clean_name, sizeof(clean_name), "%s", var_name);
    tamizhi_codegen_trim(clean_name);

    LLVMValueRef ret_llvm_val = NULL;

    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, "__tamizhi_ret") == 0) {
            ret_llvm_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "ret_val_load");
            break;
        }
    }

    if (!ret_llvm_val) {
        ret_llvm_val = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    }

    LLVMValueRef target_ptr = NULL;
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, clean_name) == 0) {
            target_ptr = symbol_table[i].alloca_ptr;
            break;
        }
    }

    if (!target_ptr && var_count < MAX_VARS) {
        if (current_function == NULL) {
            current_function = LLVMGetNamedFunction(module, "main");
        }
        target_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), clean_name);
        snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_name);
        symbol_table[var_count].alloca_ptr = target_ptr;
        symbol_table[var_count].is_str_type = 0;
        var_count++;
    }

    if (target_ptr) {
        LLVMBuildStore(builder, ret_llvm_val, target_ptr);
    }
}
