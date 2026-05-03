#include "codegen.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Target.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// வெளிப்புற DNA என்கோடிங் பங்க்ஷன்கள்
extern void encode_logic(const char* input_path, const char* output_path);

// LLVM குளோபல் வேரியபிள்கள்
LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMTargetMachineRef target_machine; 
LLVMTypeRef printf_type;
LLVMValueRef printf_func;
LLVMValueRef i_ptr = NULL; 

LLVMBasicBlockRef then_block, else_block, merge_block;
LLVMBasicBlockRef loop_cond, loop_body, loop_after;

typedef struct {
    char name[50];
    LLVMValueRef alloca_ptr;
} Variable;

Variable symbol_table[100];
int var_count = 0;

// 1. மெஷின் கோடை DNA-வாக மாற்றும் பங்க்ஷன்
void tamizhi_binary_to_dna_storage(const char* filename) {
    char dna_path[2048];
    sprintf(dna_path, "./storage/project_binary.dna");
    fprintf(stderr, " [DNA-VM] Converting Machine Code to DNA Sequence...\n");
    encode_logic(filename, dna_path); 
    fprintf(stderr, " [DNA-VM] Binary AOT Secured at: %s\n", dna_path);
}

// 2. கம்பைலர் இனிஷியலைசேஷன்
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
}

void tamizhi_generate_entry() {
    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
}

// 3. வேரியபிள் மேனேஜ்மென்ட்
void tamizhi_gen_var(char* name, int value) {
    if (var_count >= 100) return;
    LLVMValueRef alloca = LLVMBuildAlloca(builder, LLVMInt32Type(), name);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), alloca);
    strcpy(symbol_table[var_count].name, name);
    symbol_table[var_count].alloca_ptr = alloca;
    var_count++;
}

void tamizhi_gen_var_add(char* res_name, char* var1, char* var2) {
    LLVMValueRef v1_ptr = NULL, v2_ptr = NULL;
    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, var1) == 0) v1_ptr = symbol_table[i].alloca_ptr;
        if(strcmp(symbol_table[i].name, var2) == 0) v2_ptr = symbol_table[i].alloca_ptr;
    }
    if(v1_ptr && v2_ptr) {
        LLVMValueRef val1 = LLVMBuildLoad2(builder, LLVMInt32Type(), v1_ptr, "v1");
        LLVMValueRef val2 = LLVMBuildLoad2(builder, LLVMInt32Type(), v2_ptr, "v2");
        LLVMValueRef sum = LLVMBuildAdd(builder, val1, val2, "sum_tmp");
        LLVMValueRef res_ptr = LLVMBuildAlloca(builder, LLVMInt32Type(), res_name);
        LLVMBuildStore(builder, sum, res_ptr);
        strcpy(symbol_table[var_count].name, res_name);
        symbol_table[var_count].alloca_ptr = res_ptr;
        var_count++;
    }
}

// 4. லாஜிக் கன்ட்ரோல் (Print, If, Else)
void tamizhi_gen_print(char* var_name) {
    LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt");
    LLVMValueRef val = NULL;
    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, var_name) == 0) {
            val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "load_val");
            break;
        }
    }
    if(val) {
        LLVMValueRef args[] = { fmt, val };
        LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
    }
}

// விடுபட்ட If-Else லாஜிக்
void tamizhi_gen_else_start() {
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) {
        LLVMBuildBr(builder, merge_block);
    }
    LLVMPositionBuilderAtEnd(builder, else_block);
}

void tamizhi_gen_if_end() {
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) {
        LLVMBuildBr(builder, merge_block);
    }
    LLVMPositionBuilderAtEnd(builder, merge_block);
}

// 5. இறுதி கட்டம்: Binary Generation
void tamizhi_codegen_finish() {
    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));

    char *error = NULL;
    const char *out_file = "output.o";

    if (target_machine && LLVMTargetMachineEmitToFile(target_machine, module, (char*)out_file, LLVMObjectFile, &error)) {
        fprintf(stderr, " [Codegen Error] Failed to emit machine code: %s\n", error);
        LLVMDisposeMessage(error);
        return;
    }

    tamizhi_binary_to_dna_storage(out_file);
    remove(out_file);
    fprintf(stderr, "\n[Codegen] --- Tamizhi Binary DNA Engine: SUCCESS ---\n");
    LLVMDisposeBuilder(builder);
}
