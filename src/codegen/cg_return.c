#include "codegen_bridge.h"

void tamizhi_gen_return(char* return_val) {
    char clean_name[1024];
    snprintf(clean_name, sizeof(clean_name), "%s", return_val);
    tamizhi_codegen_trim(clean_name);

    LLVMValueRef ret_llvm_val = NULL;

    if (isdigit((unsigned char)clean_name[0]) || clean_name[0] == '-') {
        ret_llvm_val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_name), 0);
    } 
    else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, clean_name) == 0) {
                ret_llvm_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "ret_load");
                break;
            }
        }
    }

    if (!ret_llvm_val) {
        ret_llvm_val = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    }

    LLVMValueRef target_ptr = NULL;
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, "__tamizhi_ret") == 0) {
            target_ptr = symbol_table[i].alloca_ptr;
            break;
        }
    }

    if (!target_ptr && var_count < MAX_VARS) {
        if (current_function == NULL) {
            current_function = LLVMGetNamedFunction(module, "main");
        }
        target_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), "__tamizhi_ret");
        snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "__tamizhi_ret");
        symbol_table[var_count].alloca_ptr = target_ptr;
        symbol_table[var_count].is_str_type = 0;
        var_count++;
    }

    if (target_ptr) {
        LLVMBuildStore(builder, ret_llvm_val, target_ptr);
    }
}
