#include "codegen_bridge.h"

int call_depth = 0; 
int var_count = 0;

// ஸ்ட்ரிங் மாறிகளை (String Variables) உருவாக்குவதற்கான புதிய ஃபங்ஷன்
void tamizhi_gen_str_var(char* name, char* str_value) {
    char clean_name[100];
    snprintf(clean_name, sizeof(clean_name), "%s", name);
    tamizhi_codegen_trim(clean_name);

    // ஸ்ட்ரிங்கில் உள்ள இரட்டை மேற்கோள்களை (Quotes) நீக்குதல்
    char clean_val[1024];
    int len = strlen(str_value);
    if (str_value[0] == '"' && str_value[len - 1] == '"' && len >= 2) {
        memset(clean_val, 0, sizeof(clean_val));
        strncpy(clean_val, str_value + 1, len - 2);
    } else {
        snprintf(clean_val, sizeof(clean_val), "%s", str_value);
    }

    if (current_function == NULL) {
        current_function = LLVMGetNamedFunction(module, "main");
    }

    // 1. LLVM-ல் ஸ்ட்ரிங்கிற்கான பாயிண்டர் மெமரியை உருவாக்குதல் (i8*)
    LLVMTypeRef str_ptr_type = LLVMPointerType(LLVMInt8TypeInContext(context), 0);
    LLVMValueRef alloca = create_entry_alloca(current_function, str_ptr_type, clean_name);

    // 2. Global String Literal-ஐ உருவாக்குதல்
    LLVMValueRef global_str = LLVMBuildGlobalStringPtr(builder, clean_val, "global_str");

    // 3. அந்த ஸ்ட்ரிங்கை ஒதுக்கப்பட்ட மெமரியில் ஸ்டோர் செய்தல்
    LLVMBuildStore(builder, global_str, alloca);

    // 4. Symbol Table-ல் பதிவு செய்தல் 
    snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_name);
    symbol_table[var_count].alloca_ptr = alloca;
    symbol_table[var_count].is_str_type = 1;  
    symbol_table[var_count].has_static_val = 0; 

    // 🌟 THE MASTER FIX: இது எந்த Scope-ல் (Global/Local) வருகிறது என்பதை பதிவு செய்கிறோம்!
    symbol_table[var_count].scope_depth = call_depth; 

    var_count++;
}
