/*#include "codegen.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <stdio.h>
#include <stdlib.h>

LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMTypeRef printf_type;
LLVMValueRef printf_func;
LLVMValueRef i_ptr; // Global pointer for the loop variable

void tamizhi_codegen_init() {
    module = LLVMModuleCreateWithName("tamizhi_engine");
    builder = LLVMCreateBuilder();

    // Define printf type: int printf(char*, ...)
    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    printf_type = LLVMFunctionType(LLVMInt32Type(), printf_args, 1, 1);
    printf_func = LLVMAddFunction(module, "printf", printf_type);

    fprintf(stderr," [Codegen] LLVM Engine initialized.\n");
}

void tamizhi_generate_entry() {
    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);

    LLVMSetFunctionCallConv(main_func, LLVMCCallConv); 
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
}

void tamizhi_gen_print(char* var_name) {
    LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt");
    // Load current value from the global i_ptr
    LLVMValueRef val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "load_val");
    
    LLVMValueRef args[] = { fmt, val };
    LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
}

void tamizhi_gen_loop_test(int limit) {
    LLVMValueRef main_func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(main_func, "loop_cond");
    LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(main_func, "loop_body");
    LLVMBasicBlockRef after_block = LLVMAppendBasicBlock(main_func, "loop_after");

    // FIX: Removed 'LLVMValueRef' prefix to assign to the GLOBAL i_ptr
    i_ptr = LLVMBuildAlloca(builder, LLVMInt32Type(), "i");
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), 0, 0), i_ptr);

    LLVMBuildBr(builder, cond_block);
    LLVMPositionBuilderAtEnd(builder, cond_block);

    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "i_val");
    LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntSLT, i_val, LLVMConstInt(LLVMInt32Type(), limit, 0), "tmp_cond");
    LLVMBuildCondBr(builder, cond, body_block, after_block);

    LLVMPositionBuilderAtEnd(builder, body_block);
    
    // Call print inside the loop body so we actually see output
    tamizhi_gen_print("i");

    LLVMValueRef next_val = LLVMBuildAdd(builder, i_val, LLVMConstInt(LLVMInt32Type(), 1, 0), "next_i");
    LLVMBuildStore(builder, next_val, i_ptr);
    LLVMBuildBr(builder, cond_block);

    LLVMPositionBuilderAtEnd(builder, after_block);
}

void tamizhi_codegen_finish() {
    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0)); 
    
    char *ir_string = LLVMPrintModuleToString(module);
    printf("%s", ir_string); 
    LLVMDisposeMessage(ir_string);
    LLVMDisposeBuilder(builder);
}
// இரண்டு எண்களைக் கூட்டி அச்சிடும் லாஜிக்
void tamizhi_gen_add_and_print(int n1, int n2) {
    // 1. கூட்டல் செய்ய (n1 + n2)
    LLVMValueRef val1 = LLVMConstInt(LLVMInt32Type(), n1, 0);
    LLVMValueRef val2 = LLVMConstInt(LLVMInt32Type(), n2, 0);
    LLVMValueRef sum = LLVMBuildAdd(builder, val1, val2, "tmp_sum");
    
    // 2. பிரிண்ட் செய்ய பார்மேட் ஸ்ட்ரிங்
    LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt_sum");
    
    // 3. பிரிண்ட் பங்க்ஷனை அழைத்தல்
    LLVMValueRef args[] = { fmt, sum };
    LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_sum_call");
}
*/
#include "codegen.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMTypeRef printf_type;
LLVMValueRef printf_func;
LLVMValueRef i_ptr; 

// --- சிம்பிள் Symbol Table ---
typedef struct {
    char name[50];
    LLVMValueRef alloca_ptr;
} Variable;

Variable symbol_table[100];
int var_count = 0;

void tamizhi_codegen_init() {
    module = LLVMModuleCreateWithName("tamizhi_engine");
    builder = LLVMCreateBuilder();

    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    printf_type = LLVMFunctionType(LLVMInt32Type(), printf_args, 1, 1);
    printf_func = LLVMAddFunction(module, "printf", printf_type);

    fprintf(stderr," [Codegen] LLVM Engine initialized.\n");
}

void tamizhi_generate_entry() {
    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
}

// --- புதிய 'முழுஎண்' (Variable) லாஜிக் ---
void tamizhi_gen_var(char* name, int value) {
    // 1. மெமரி ஒதுக்கு (Alloca)
    LLVMValueRef alloca = LLVMBuildAlloca(builder, LLVMInt32Type(), name);
    
    // 2. மதிப்பை உள்ளே போடு (Store)
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), alloca);
    
    // 3. பெயர் பட்டியலில் சேமி (Symbol Table)
    strcpy(symbol_table[var_count].name, name);
    symbol_table[var_count].alloca_ptr = alloca;
    var_count++;

    fprintf(stderr, "[Codegen] Variable '%s' = %d stored in memory.\n", name, value);
}

void tamizhi_gen_print(char* var_name) {
    LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt");
    LLVMValueRef val;

    // Symbol Table-ல் தேடி அந்த மாறியின் மதிப்பை எடுப்போம்
    int found = 0;
    for(int i=0; i<var_count; i++) {
        if(strcmp(symbol_table[i].name, var_name) == 0) {
            val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "load_val");
            found = 1;
            break;
        }
    }

    // ஒருவேளை லூப் மாறியாக இருந்தால் (பழைய i_ptr லாஜிக்)
    if(!found) val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "load_val");
    
    LLVMValueRef args[] = { fmt, val };
    LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
}

void tamizhi_gen_loop_test(int limit) {
    LLVMValueRef main_func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
    LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(main_func, "loop_cond");
    LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(main_func, "loop_body");
    LLVMBasicBlockRef after_block = LLVMAppendBasicBlock(main_func, "loop_after");

    i_ptr = LLVMBuildAlloca(builder, LLVMInt32Type(), "i");
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), 0, 0), i_ptr);

    LLVMBuildBr(builder, cond_block);
    LLVMPositionBuilderAtEnd(builder, cond_block);

    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "i_val");
    LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntSLT, i_val, LLVMConstInt(LLVMInt32Type(), limit, 0), "tmp_cond");
    LLVMBuildCondBr(builder, cond, body_block, after_block);

    LLVMPositionBuilderAtEnd(builder, body_block);
    tamizhi_gen_print("i");

    LLVMValueRef next_val = LLVMBuildAdd(builder, i_val, LLVMConstInt(LLVMInt32Type(), 1, 0), "next_i");
    LLVMBuildStore(builder, next_val, i_ptr);
    LLVMBuildBr(builder, cond_block);

    LLVMPositionBuilderAtEnd(builder, after_block);
}

void tamizhi_codegen_finish() {
    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0)); 
    char *ir_string = LLVMPrintModuleToString(module);
    printf("%s", ir_string); 
    LLVMDisposeMessage(ir_string);
    LLVMDisposeBuilder(builder);
}

void tamizhi_gen_add_and_print(int n1, int n2) {
    LLVMValueRef val1 = LLVMConstInt(LLVMInt32Type(), n1, 0);
    LLVMValueRef val2 = LLVMConstInt(LLVMInt32Type(), n2, 0);
    LLVMValueRef sum = LLVMBuildAdd(builder, val1, val2, "tmp_sum");
    LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt_sum");
    LLVMValueRef args[] = { fmt, sum };
    LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_sum_call");
}
