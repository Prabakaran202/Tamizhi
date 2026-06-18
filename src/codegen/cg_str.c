#include "codegen_bridge.h"

void tamizhi_gen_str(char* name, char* value) {
    char clean_res[100];
    snprintf(clean_res, sizeof(clean_res), "%s", name);
    tamizhi_codegen_trim(clean_res);

    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, clean_res) == 0) {
            LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(builder, value, "str_lit");
            symbol_table[i].alloca_ptr = str_ptr;
            symbol_table[i].is_str_type = 1;
            symbol_table[i].has_static_val = 0;
            return;
        }
    }
    LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(builder, value, "str_lit");
    snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
    symbol_table[var_count].alloca_ptr = str_ptr; 
    symbol_table[var_count].is_str_type = 1;
    symbol_table[var_count].has_static_val = 0;
    var_count++;
}
