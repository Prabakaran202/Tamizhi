#include "codegen_bridge.h"

void tamizhi_gen_var(char* name, int value) {
    char clean_res[100];
    snprintf(clean_res, sizeof(clean_res), "%s", name);
    tamizhi_codegen_trim(clean_res);

    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, clean_res) == 0) {
            symbol_table[i].static_val = value;
            symbol_table[i].has_static_val = 1;
            symbol_table[i].is_str_type = 0;
            if(symbol_table[i].alloca_ptr) {
                LLVMBuildStore(builder, LLVMConstInt(LLVMInt32TypeInContext(context), value, 0), symbol_table[i].alloca_ptr);
            }
            return;
        }
    }

    if (current_function == NULL) {
        current_function = LLVMGetNamedFunction(module, "main");
    }
    LLVMValueRef alloca = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), clean_res);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32TypeInContext(context), value, 0), alloca);

    snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
    symbol_table[var_count].alloca_ptr = alloca;
    symbol_table[var_count].is_str_type = 0;
    symbol_table[var_count].static_val = value;
    symbol_table[var_count].has_static_val = 1;
    var_count++;
}
