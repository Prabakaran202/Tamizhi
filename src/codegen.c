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

// 1. Universal Bitcode & Assembly உருவாக்கும் பங்க்ஷன்
void tamizhi_generate_universal_bitcode(const char* filename) {
    // A. LLVM Bitcode (.bc) உருவாக்குதல்
    if (LLVMWriteBitcodeToFile(module, filename) != 0) {
        fprintf(stderr, " [Error] Failed to write universal bitcode!\n");
    } else {
        fprintf(stderr, " [Universal] Bitcode generated: %s\n", filename);
    }

    // B. LLVM Assembly (.ll) உருவாக்குதல் (மனிதர்கள் படிக்கக்கூடியது)
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

void tamizhi_generate_entry() {
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

void tamizhi_gen_print(char* var_name) {
    LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt");
    LLVMValueRef val = NULL;
    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, var_name) == 0) {
            val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "load_val");
            break;
        }
    }
    if(!val && i_ptr && strcmp(var_name, "i") == 0) val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "load_val");
    if(val) {
        LLVMValueRef args[] = { fmt, val };
        LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
    }
}

// 5. லாஜிக் கன்ட்ரோல்
void tamizhi_gen_if_start(char* var1, char* op, char* var2) {
    LLVMValueRef v1 = NULL, v2 = NULL;
    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, var1) == 0) v1 = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "v1");
    }
    if(isdigit(var2[0])) v2 = LLVMConstInt(LLVMInt32Type(), atoi(var2), 0);
    else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, var2) == 0) v2 = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "v2");
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
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) LLVMBuildBr(builder, merge_block);
    LLVMPositionBuilderAtEnd(builder, else_block);
}

void tamizhi_gen_if_end() {
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) LLVMBuildBr(builder, merge_block);
    LLVMPositionBuilderAtEnd(builder, merge_block);
}

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

// 6. இறுதி கட்டம்
void tamizhi_codegen_finish() {
    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));

    // Universal Files (bc & ll)
    tamizhi_generate_universal_bitcode("output.bc");

    // Machine Code (Object file)
    char *error = NULL;
    const char *out_file = "output.o";
    if (target_machine && LLVMTargetMachineEmitToFile(target_machine, module, (char*)out_file, LLVMObjectFile, &error)) {
        fprintf(stderr, " [Codegen Error] Failed to emit machine code: %s\n", error);
        LLVMDisposeMessage(error);
    }

    tamizhi_binary_to_dna_storage(out_file);
    remove(out_file);

    fprintf(stderr, "\n[Codegen] --- Tamizhi Universal Engine: SUCCESS ---\n");
    LLVMDisposeBuilder(builder);
}
