#include "codegen_bridge.h"

void tamizhi_gen_print(char* var_name) {
    LLVMValueRef val = NULL;
    int is_string = 0;
    char clean_name[1024];
    snprintf(clean_name, sizeof(clean_name), "%s", var_name);
    tamizhi_codegen_trim(clean_name);

    int is_literal = 0;
    int len = strlen(clean_name);
    if (clean_name[0] == '"' && clean_name[len - 1] == '"' && len >= 2) {
        char temp[1024];
        memset(temp, 0, sizeof(temp));
        strncpy(temp, clean_name + 1, len - 2);
        strcpy(clean_name, temp);
        is_literal = 1;
        is_string = 1;
    }

    if (!is_literal) {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, clean_name) == 0) {
                val = symbol_table[i].alloca_ptr;
                if (symbol_table[i].is_str_type) {
                    is_string = 1;
                } else {
                    val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), val, "load_val");
                }
                break;
            }
        }
    }

    if (!val && strcmp(clean_name, "i") == 0) {
        for (int lvl = loop_top; lvl >= 0; lvl--) {
            if (loop_stack[lvl].i_ptr) {
                val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), loop_stack[lvl].i_ptr, "load_loop_i");
                break;
            }
        }
    }

    if(!val && (isdigit((unsigned char)clean_name[0]) || clean_name[0] == '-') && !is_literal) {
        val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_name), 0);
    }

    if(!val) {
        val = LLVMBuildGlobalStringPtr(builder, clean_name, "str_lit");
        is_string = 1;
    }

    if(val) {
        const char* fmt = is_string ? "%s\n" : "%d\n";
        LLVMValueRef fmt_ref = LLVMBuildGlobalStringPtr(builder, fmt, "fmt");
        LLVMValueRef args[] = { fmt_ref, val };
        LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
    }
}
