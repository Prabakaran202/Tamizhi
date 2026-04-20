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

// --- வேரியபிள் கூட்டல் லாஜிக் (New!) ---
void tamizhi_gen_var_add(char* res_name, char* var1, char* var2) {
    LLVMValueRef v1_ptr = NULL, v2_ptr = NULL;

    // Symbol Table-ல் var1 மற்றும் var2-வை தேடுவோம்
    for(int i=0; i<var_count; i++) {
        if(strcmp(symbol_table[i].name, var1) == 0) v1_ptr = symbol_table[i].alloca_ptr;
        if(strcmp(symbol_table[i].name, var2) == 0) v2_ptr = symbol_table[i].alloca_ptr;
    }

    if(v1_ptr && v2_ptr) {
        // மெமரில இருந்து மதிப்புகளை எடு (Load)
        LLVMValueRef val1 = LLVMBuildLoad2(builder, LLVMInt32Type(), v1_ptr, "v1");
        LLVMValueRef val2 = LLVMBuildLoad2(builder, LLVMInt32Type(), v2_ptr, "v2");
        
        // கூட்டல் செய் (Add)
        LLVMValueRef sum = LLVMBuildAdd(builder, val1, val2, "sum_tmp");

        // முடிவை புதிய மாறியில் (res_name) சேமி (Alloca & Store)
        LLVMValueRef res_ptr = LLVMBuildAlloca(builder, LLVMInt32Type(), res_name);
        LLVMBuildStore(builder, sum, res_ptr);

        // Symbol Table-ல் புதிய மாறியைச் சேர்ப்போம்
        strcpy(symbol_table[var_count].name, res_name);
        symbol_table[var_count].alloca_ptr = res_ptr;
        var_count++;
        
        fprintf(stderr, "[Codegen] Logic: %s = %s + %s completed.\n", res_name, var1, var2);
    } else {
        fprintf(stderr, "[Codegen] Error: Variables %s or %s not found!\n", var1, var2);
    }
}

void tamizhi_gen_var(char* name, int value) {
    LLVMValueRef alloca = LLVMBuildAlloca(builder, LLVMInt32Type(), name);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), alloca);
    
    strcpy(symbol_table[var_count].name, name);
    symbol_table[var_count].alloca_ptr = alloca;
    var_count++;

    fprintf(stderr, "[Codegen] Variable '%s' = %d stored.\n", name, value);
}

void tamizhi_gen_print(char* var_name) {
    LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt");
    LLVMValueRef val = NULL;

    for(int i=0; i<var_count; i++) {
        if(strcmp(symbol_table[i].name, var_name) == 0) {
            val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "load_val");
            break;
        }
    }

    if(!val) val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "load_val");
    
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
