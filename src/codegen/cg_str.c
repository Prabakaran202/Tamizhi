#include "codegen_bridge.h"

void tamizhi_gen_str(char* name, char* value) {
    char clean_res[100];
    snprintf(clean_res, sizeof(clean_res), "%s", name);
    tamizhi_codegen_trim(clean_res);

    // 1. 🌟 Global String-ஐ ஒரு முறை மட்டும் பாதுகாப்பாக உருவாக்குகிறோம்
    LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(builder, value, "str_lit");

    // 2. 🌟 ஏற்கனவே வேரியபிள் இருக்கிறதா என்று செக் செய்கிறோம்
    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, clean_res) == 0) {
            symbol_table[i].alloca_ptr = str_ptr;
            symbol_table[i].is_str_type = 1;
            symbol_table[i].has_static_val = 0;
            return;
        }
    }

    // 3. 🌟 மாஸ்டர் ஃபிக்ஸ்: Memory Overflow வராமல் தடுக்க Bounds Check!
    if (var_count >= MAX_VARS) {
        fprintf(stderr, "[Memory Error] Maximum variable limit reached!\n");
        return;
    }

    // 4. 🌟 புதிதாக டேபிளில் இணைக்கிறோம்
    snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
    symbol_table[var_count].alloca_ptr = str_ptr; 
    symbol_table[var_count].is_str_type = 1;
    symbol_table[var_count].has_static_val = 0;
    var_count++;
}
