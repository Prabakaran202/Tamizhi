#include "codegen.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <ctype.h>

// --- DNA-VM லாஜிக் அறிவிப்புகள் ---
extern void encode_logic(const char* input_path, const char* output_path);
extern void decode_logic(const char* dna_path, const char* output_path);

// 🌍 குளோபல் வேரியபிள்கள்
LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMTypeRef printf_type;
LLVMValueRef printf_func;
LLVMValueRef i_ptr = NULL; 

// இஃப்-எல்ஸ் மற்றும் லூப் பிளாக்குகளுக்கான குளோபல் ரெஃபரன்ஸ்
LLVMBasicBlockRef then_block, else_block, merge_block;
LLVMBasicBlockRef loop_cond, loop_body, loop_after;

typedef struct {
    char name[50];
    LLVMValueRef alloca_ptr;
} Variable;

Variable symbol_table[100];
int var_count = 0;

// 1. DNA-VM: Absolute Path Storage
void tamizhi_dna_secure_storage(char* name, int value) {
    char temp_raw[100], dna_file[2048];
    sprintf(temp_raw, "temp_%s.txt", name);
    sprintf(dna_file, "/data/data/com.termux/files/home/Tamizhi/storage/%s.dna", name);

    FILE *f = fopen(temp_raw, "w");
    if (f) {
        fprintf(f, "%d", value);
        fclose(f);
        encode_logic(temp_raw, dna_file);
        remove(temp_raw);
        fprintf(stderr, " [DNA-VM] Secured at: %s\n", dna_file);
    }
}

void tamizhi_codegen_init() {
    module = LLVMModuleCreateWithName("tamizhi_engine");
    builder = LLVMCreateBuilder();

    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    printf_type = LLVMFunctionType(LLVMInt32Type(), printf_args, 1, 1);
    printf_func = LLVMAddFunction(module, "printf", printf_type);

    fprintf(stderr," [Codegen] LLVM Engine + Global Block Logic Initialized.\n");
}

void tamizhi_generate_entry() {
    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
}

void tamizhi_gen_var(char* name, int value) {
    if (var_count >= 100) return;
    tamizhi_dna_secure_storage(name, value);

    LLVMValueRef alloca = LLVMBuildAlloca(builder, LLVMInt32Type(), name);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), alloca);

    strcpy(symbol_table[var_count].name, name);
    symbol_table[var_count].alloca_ptr = alloca;
    var_count++;
}

void tamizhi_gen_print(char* var_name) {
    LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt");
    LLVMValueRef val = NULL;

    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, var_name) == 0) {
            val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "load_val");
            break;
        }
    }
    if(!val && i_ptr && strcmp(var_name, "i") == 0) {
        val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "load_val");
    }

    if(val) {
        LLVMValueRef args[] = { fmt, val };
        LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
    }
}

// 2. ⚡ 'எனில்' (If-Else) லாஜிக்
void tamizhi_gen_if_start(char* var1, char* op, char* var2) {
    LLVMValueRef v1 = NULL, v2 = NULL;
    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, var1) == 0) 
            v1 = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "v1");
    }
    
    if(isdigit(var2[0])) v2 = LLVMConstInt(LLVMInt32Type(), atoi(var2), 0);
    else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, var2) == 0)
                v2 = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "v2");
        }
    }

    if(!v1 || !v2) return;

    LLVMIntPredicate pred = (strcmp(op, "<") == 0) ? LLVMIntSLT : (strcmp(op, ">") == 0) ? LLVMIntSGT : LLVMIntEQ;
    LLVMValueRef cond = LLVMBuildICmp(builder, pred, v1, v2, "if_cond");

    LLVMValueRef func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
    then_block = LLVMAppendBasicBlock(func, "then");
    else_block = LLVMAppendBasicBlock(func, "else");
    merge_block = LLVMAppendBasicBlock(func, "if_cont");

    LLVMBuildCondBr(builder, cond, then_block, else_block);
    LLVMPositionBuilderAtEnd(builder, then_block);
}

void tamizhi_gen_else_start() {
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL)
        LLVMBuildBr(builder, merge_block);
    LLVMPositionBuilderAtEnd(builder, else_block);
}

void tamizhi_gen_if_end() {
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL)
        LLVMBuildBr(builder, merge_block);
    LLVMPositionBuilderAtEnd(builder, merge_block);
}

// 3. 🌀 'சு' (Loop) லாஜிக்
void tamizhi_gen_loop_start(int limit) {
    LLVMValueRef func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
    loop_cond = LLVMAppendBasicBlock(func, "loop_cond");
    loop_body = LLVMAppendBasicBlock(func, "loop_body");
    loop_after = LLVMAppendBasicBlock(func, "loop_after");

    i_ptr = LLVMBuildAlloca(builder, LLVMInt32Type(), "i");
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), 0, 0), i_ptr);

    LLVMBuildBr(builder, loop_cond);
    LLVMPositionBuilderAtEnd(builder, loop_cond);

    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "i_val");
    LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntSLT, i_val, LLVMConstInt(LLVMInt32Type(), limit, 0), "tmp_cond");
    LLVMBuildCondBr(builder, cond, loop_body, loop_after);

    LLVMPositionBuilderAtEnd(builder, loop_body);
}

void tamizhi_gen_loop_end() {
    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "i_val");
    LLVMValueRef next_val = LLVMBuildAdd(builder, i_val, LLVMConstInt(LLVMInt32Type(), 1, 0), "next_i");
    LLVMBuildStore(builder, next_val, i_ptr);
    LLVMBuildBr(builder, loop_cond); 
    LLVMPositionBuilderAtEnd(builder, loop_after);
}

void tamizhi_codegen_finish() {
    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0)); 
    char *ir_string = LLVMPrintModuleToString(module);
    printf("%s", ir_string); 
    LLVMDisposeMessage(ir_string);
    LLVMDisposeBuilder(builder);
}
