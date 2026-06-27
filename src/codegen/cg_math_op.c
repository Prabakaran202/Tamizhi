#include "codegen_bridge.h"

void tamizhi_gen_math_op(char* res_name, char* v1, char* op, char* var2) {
    char clean_res[100], clean_v1[100], clean_v2[100];
    snprintf(clean_res, sizeof(clean_res), "%s", res_name); tamizhi_codegen_trim(clean_res);
    snprintf(clean_v1, sizeof(clean_v1), "%s", v1); tamizhi_codegen_trim(clean_v1);
    snprintf(clean_v2, sizeof(clean_v2), "%s", var2); tamizhi_codegen_trim(clean_v2);

    LLVMValueRef v1_val = NULL, v2_val = NULL;
    int s_val1 = 0, s_val2 = 0;
    int f1 = 0, f2 = 0;

    if(isdigit((unsigned char)clean_v1[0]) || clean_v1[0] == '-') {
        v1_val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_v1), 0);
        s_val1 = atoi(clean_v1);
        f1 = 1;
    } else {
        // 🌟 THE MASTER FIX: ரிவர்ஸ் தேடல் (Reverse Search for v1)
        for(int i = var_count - 1; i >= 0; i--) {
            if(strcmp(symbol_table[i].name, clean_v1) == 0) {
                if(symbol_table[i].has_static_val) {
                    s_val1 = symbol_table[i].static_val;
                    f1 = 1;
                }
                v1_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "v1");
                break;
            }
        }
    }

    if(isdigit((unsigned char)clean_v2[0]) || clean_v2[0] == '-') {
        v2_val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_v2), 0);
        s_val2 = atoi(clean_v2);
        f2 = 1;
    } else {
        // 🌟 THE MASTER FIX: ரிவர்ஸ் தேடல் (Reverse Search for v2)
        for(int i = var_count - 1; i >= 0; i--) {
            if(strcmp(symbol_table[i].name, clean_v2) == 0) {
                if(symbol_table[i].has_static_val) {
                    s_val2 = symbol_table[i].static_val;
                    f2 = 1;
                }
                v2_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "v2");
                break;
            }
        }
    }

    if(v1_val && v2_val) {
        LLVMValueRef math_res = NULL;
        int calculated_val = 0;

        if (strcmp(op, "+") == 0) {
            math_res = LLVMBuildAdd(builder, v1_val, v2_val, "add_tmp");
            calculated_val = s_val1 + s_val2;
        } else if (strcmp(op, "-") == 0) {
            math_res = LLVMBuildSub(builder, v1_val, v2_val, "sub_tmp");
            calculated_val = s_val1 - s_val2;
        } else if (strcmp(op, "*") == 0) {
            math_res = LLVMBuildMul(builder, v1_val, v2_val, "mul_tmp");
            calculated_val = s_val1 * s_val2;
        } else if (strcmp(op, "/") == 0) {
            if(f2 && s_val2 == 0) return;
            math_res = LLVMBuildSDiv(builder, v1_val, v2_val, "div_tmp");
            if (s_val2 != 0) calculated_val = s_val1 / s_val2;
        }

        if (math_res) {
            LLVMValueRef target_ptr = NULL;
            int found_idx = -1;

            // 🌟 THE MASTER FIX: ரிவர்ஸ் தேடல் (Reverse Search for Result Variable)
            for(int i = var_count - 1; i >= 0; i--) {
                if(strcmp(symbol_table[i].name, clean_res) == 0) {
                    target_ptr = symbol_table[i].alloca_ptr;
                    found_idx = i;
                    break;
                }
            }

            if(!target_ptr && var_count < MAX_VARS) {
                if (current_function == NULL) {
                    current_function = LLVMGetNamedFunction(module, "main");
                }
                target_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), clean_res);
                snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
                symbol_table[var_count].alloca_ptr = target_ptr;
                symbol_table[var_count].is_str_type = 0;
                
                // 🌟 SCOPE DEPTH TRACKING
                symbol_table[var_count].scope_depth = call_depth; 
                
                found_idx = var_count;
                var_count++;
            }

            if(target_ptr && found_idx != -1) {
                LLVMBuildStore(builder, math_res, target_ptr);
                if(f1 && f2) {
                    symbol_table[found_idx].static_val = calculated_val;
                    symbol_table[found_idx].has_static_val = 1;
                }
            }
        }
    }
}
