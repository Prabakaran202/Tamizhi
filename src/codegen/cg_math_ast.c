#include "codegen_bridge.h"

void tamizhi_gen_math_ast(char* res_name, ASTNode* root) {
    char clean_res[100];
    snprintf(clean_res, sizeof(clean_res), "%s", res_name);
    tamizhi_codegen_trim(clean_res);

    LLVMValueRef math_res = tamizhi_evaluate_ast(root);
    if (!math_res) return;

    LLVMValueRef target_ptr = NULL;
    int found_idx = -1;

    // 🌟 STRICT LOCAL SCOPE SHADOWING
    for (int i = var_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, clean_res) == 0) {
            // ஒருவேளை இதே ஸ்கோப்ல இருந்தா மட்டும் அதை ஓவர்ரைட் பண்ணு (Update)
            if (symbol_table[i].scope_depth == call_depth) {
                target_ptr = symbol_table[i].alloca_ptr;
                found_idx = i;
            }
            break; // இல்லனா அதத் தொடாதே! புதுசா லோக்கலா கிரியேட் பண்ண விடு.
        }
    }

    // புதுசா லோக்கல் மாறி உருவாக்குதல்
    if (!target_ptr && var_count < MAX_VARS) {
        if (current_function == NULL) {
            current_function = LLVMGetNamedFunction(module, "main");
        }
        target_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), clean_res);
        snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
        symbol_table[var_count].alloca_ptr = target_ptr;
        symbol_table[var_count].is_str_type = 0;

        // எந்த ஆழத்தில் மாறிகள் உருவாகிறது?
        symbol_table[var_count].scope_depth = call_depth; 

        found_idx = var_count;
        var_count++;
    }

    if (target_ptr && found_idx != -1) {
        LLVMBuildStore(builder, math_res, target_ptr);
        symbol_table[found_idx].has_static_val = 0; 
    }
}
