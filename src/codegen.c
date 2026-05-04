#include "codegen.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Target.h>
#include <llvm-c/BitWriter.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// வெளிப்புற DNA என்கோடிங் லாஜிக்
extern void encode_logic(const char* input_path, const char* output_path);

// LLVM மற்றும் குளோபல் வேரியபிள்கள்
LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMTargetMachineRef target_machine = NULL; 
LLVMTypeRef printf_type;
LLVMValueRef printf_func;
LLVMValueRef i_ptr = NULL; 

LLVMBasicBlockRef then_block, else_block, merge_block;
LLVMBasicBlockRef loop_cond, loop_body, loop_after;

// சிம்பல் டேபிள்
typedef struct {
    char name[50];
    LLVMValueRef alloca_ptr;
} Variable;

Variable symbol_table[100];
int var_count = 0;

// 1. Universal Bitcode & Assembly உருவாக்கம்
void tamizhi_generate_universal_bitcode(const char* filename) {
    if (LLVMWriteBitcodeToFile(module, filename) != 0) {
        fprintf(stderr, " [Error] Failed to write universal bitcode!\n");
    } else {
        fprintf(stderr, " [Universal] Bitcode generated: %s\n", filename);
    }

    char asm_path[256];
    sprintf(asm_path, "output.ll");
    FILE *f = fopen(asm_path, "w");
    if (f) {
        char *str = LLVMPrintModuleToString(module);
        fprintf(f, "%s", str);
        LLVMDisposeMessage(str);
        fclose(f);
        fprintf(stderr, " [Universal] LLVM Assembly generated: %s\n", asm_path);
    }
}

// 2. DNA VM Storage
void tamizhi_binary_to_dna_storage(const char* filename) {
    char dna_path[2048];
    sprintf(dna_path, "storage/project_binary.dna");

    FILE *check = fopen(filename, "rb");
    if (check) {
        fclose(check);
        encode_logic(filename, dna_path); 
        fprintf(stderr, " [DNA-VM] Binary AOT Secured at: %s\n", dna_path);
    } else {
        fprintf(stderr, " [DNA-VM Error] Machine code file (%s) not found for encoding!\n", filename);
    }
}

// 3. கம்பைலர் இனிஷியலைசேஷன்
void tamizhi_codegen_init() {
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllAsmPrinters();

    module = LLVMModuleCreateWithName("tamizhi_engine");
    builder = LLVMCreateBuilder();

    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    printf_type = LLVMFunctionType(LLVMInt32Type(), printf_args, 1, 1);
    printf_func = LLVMAddFunction(module, "printf", printf_type);

    fprintf(stderr, " [Codegen] LLVM Engine Initialized.\n");
}

/* மாற்றம்: ஏற்கனவே மெயின் பங்க்ஷன் இருக்கிறதா என்று செக் பண்ணும் லாஜிக்.
   இது @main.1 உருவாகாமல் தடுத்து, ஒரே ஒரு குளோபல் மெயின் பங்க்ஷனை உறுதி செய்யும்.
*/
void tamizhi_generate_entry() {
    if (LLVMGetNamedFunction(module, "main")) {
        return; 
    }

    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
}

// 4. வேரியபிள் மற்றும் கணித செயல்பாடுகள்
void tamizhi_gen_var(char* name, int value) {
    if (var_count >= 100) return;
    LLVMValueRef alloca = LLVMBuildAlloca(builder, LLVMInt32Type(), name);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), alloca);
    strcpy(symbol_table[var_count].name, name);
    symbol_table[var_count].alloca_ptr = alloca;
    var_count++;
}

void tamizhi_gen_var_add(char* res_name, char* var1, char* var2) {
    LLVMValueRef v1_val = NULL, v2_val = NULL;

    if(isdigit(var1[0])) v1_val = LLVMConstInt(LLVMInt32Type(), atoi(var1), 0);
    else {
        for(int i=0; i<var_count; i++) 
            if(strcmp(symbol_table[i].name, var1) == 0) v1_val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "v1");
    }

    if(isdigit(var2[0])) v2_val = LLVMConstInt(LLVMInt32Type(), atoi(var2), 0);
    else {
        for(int i=0; i<var_count; i++) 
            if(strcmp(symbol_table[i].name, var2) == 0) v2_val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "v2");
    }

    if(v1_val && v2_val) {
        LLVMValueRef sum = LLVMBuildAdd(builder, v1_val, v2_val, "sum_tmp");
        LLVMValueRef res_ptr = LLVMBuildAlloca(builder, LLVMInt32Type(), res_name);
        LLVMBuildStore(builder, sum, res_ptr);
        strcpy(symbol_table[var_count].name, res_name);
        symbol_table[var_count].alloca_ptr = res_ptr;
        var_count++;
    }
}

void tamizhi_gen_print(char* var_name) {
    LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt");
    LLVMValueRef val = NULL;

    if (isdigit(var_name[0])) {
        val = LLVMConstInt(LLVMInt32Type(), atoi(var_name), 0);
    } 
    else {
        for(int i = 0
