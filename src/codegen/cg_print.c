#include "codegen_bridge.h"

void tamizhi_gen_print(char* var_name) {
    LLVMValueRef val = NULL;
    int is_string = 0;
    char clean_name[1024];
    snprintf(clean_name, sizeof(clean_name), "%s", var_name);
    tamizhi_codegen_trim(clean_name);

    int is_literal = 0;
    int len = strlen(clean_name);

    // 1. ஸ்ட்ரிங் லிட்ரல்களை சரிபார்த்தல் ("தமிழி மொழி")
    if (clean_name[0] == '"' && clean_name[len - 1] == '"' && len >= 2) {
        char temp[1024];
        memset(temp, 0, sizeof(temp));
        strncpy(temp, clean_name + 1, len - 2);
        strcpy(clean_name, temp);
        is_literal = 1;
        is_string = 1;
    }

    // 2. Symbol Table-ல் மாறிகளைத் தேடுதல்
    if (!is_literal) {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, clean_name) == 0) {
                // [FIX] ஸ்ட்ரிங் மற்றும் எண்களுக்கான சரியான Load லாஜிக்
                if (symbol_table[i].is_str_type) {
                    is_string = 1;
                    // 🌟 ஸ்ட்ரிங் ஏற்கனவே Pointer ஆகத்தான் உள்ளது. 
                    // எனவே Load செய்யக்கூடாது! நேரடியாக எடுத்துக்கொள்ள வேண்டும்.
                    val = symbol_table[i].alloca_ptr; 
                } else {
                    // எண்களுக்கு (Numbers) மட்டும் i32 மதிப்பாக லோட் செய்ய வேண்டும்
                    val = symbol_table[i].alloca_ptr;
                    val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), val, "load_val");
                }
                break;
            }
        }
    }

    // 3. லூப் வேரியபிள் 'i'-ஐ தேடுதல்
    if (!val && strcmp(clean_name, "i") == 0) {
        for (int lvl = loop_top; lvl >= 0; lvl--) {
            if (loop_stack[lvl].i_ptr) {
                val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), loop_stack[lvl].i_ptr, "load_loop_i");
                break;
            }
        }
    }

    // 4. நேரடி எண்கள்
    if(!val && (isdigit((unsigned char)clean_name[0]) || clean_name[0] == '-') && !is_literal) {
        val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_name), 0);
    }

    // 5. இது எதுவுமே இல்லை என்றால் லிட்ரலாக எடுத்துக்கொள்ளுதல்
    if(!val) {
        val = LLVMBuildGlobalStringPtr(builder, clean_name, "str_lit");
        is_string = 1;
    }

    // 6. பிரிண்ட் செய்தல்
    if(val) {
        const char* fmt = is_string ? "%s\n" : "%d\n";
        LLVMValueRef fmt_ref = LLVMBuildGlobalStringPtr(builder, fmt, "fmt");
        LLVMValueRef args[] = { fmt_ref, val };
        LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
    }
}
