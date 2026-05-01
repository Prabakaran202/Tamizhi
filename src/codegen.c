#include "codegen.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h> // dirname() பங்க்ஷனுக்காக

// --- DNA-VM லாஜிக் அறிவிப்புகள் ---
extern void encode_logic(const char* input_path, const char* output_path);
extern void decode_logic(const char* dna_path, const char* output_path);

LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMTypeRef printf_type;
LLVMValueRef printf_func;
LLVMValueRef i_ptr = NULL; 

typedef struct {
    char name[50];
    LLVMValueRef alloca_ptr;
} Variable;

Variable symbol_table[100];
int var_count = 0;

// தமிழியின் மையக் களஞ்சியப் பாதையைக் கண்டுபிடிக்கும் பங்க்ஷன்
void get_tamizhi_storage_path(char* var_name, char* output_path) {
    char exe_path[1024];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
    if (len != -1) {
        exe_path[len] = '\0';
        char* dir = dirname(exe_path); 
        sprintf(output_path, "%s/storage/%s.dna", dir, var_name);
    } else {
        sprintf(output_path, "storage/%s.dna", var_name);
    }
}

void tamizhi_codegen_init() {
    module = LLVMModuleCreateWithName("tamizhi_engine");
    builder = LLVMCreateBuilder();

    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    printf_type = LLVMFunctionType(LLVMInt32Type(), printf_args, 1, 1);
    printf_func = LLVMAddFunction(module, "printf", printf_type);

    fprintf(stderr," [Codegen] LLVM Engine initialized with Binary-Relative DNA Storage.\n");
}

void tamizhi_generate_entry() {
    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
}

void tamizhi_gen_var(char* name, int value) {
    if (var_count >= 100) return;

    char dna_file[2048];
    char temp_val[100];

    // மையக் களஞ்சியப் பாதையைப் பெறுதல்
    get_tamizhi_storage_path(name, dna_file);
    sprintf(temp_val, "temp_%s.txt", name);

    // DNA Encoding லாஜிக்
    FILE *f = fopen(temp_val, "w");
    if(f) {
        fprintf(f, "%d", value);
        fclose(f);
        encode_logic(temp_val, dna_file); 
        remove(temp_val); 
    }

    LLVMValueRef alloca = LLVMBuildAlloca(builder, LLVMInt32Type(), name);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), alloca);

    strcpy(symbol_table[var_count].name, name);
    symbol_table[var_count].alloca_ptr = alloca;
    var_count++;

    fprintf(stderr, " [Storage] Variable '%s' secured at Tamizhi Core: %s\n", name, dna_file);
}

void tamizhi_gen_print(char* var_name) {
    LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt");
    LLVMValueRef val = NULL;
    char dna_file[2048];

    // 1. மெமரியில் தேடுதல்
    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, var_name) == 0) {
            val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "load_val");
            break;
        }
    }

    // 2. லூப் வேரியபிள் செக்
    if(!val && i_ptr && strcmp(var_name, "i") == 0) {
        val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "load_val");
    }

    // 3. மையக் களஞ்சியத்திலிருந்து DNA மீட்டெடுத்தல் (Auto-Recovery)
    if (!val) {
        get_tamizhi_storage_path(var_name, dna_file);

        if (access(dna_file, F_OK) == 0) { 
            fprintf(stderr, " [DNA-VM] '%s' மெமரியில் இல்லை. மையக் களஞ்சியத்திலிருந்து மீட்டெடுக்கப்படுகிறது...\n", var_name);
            decode_logic(dna_file, "temp_recovery.txt");

            FILE *res = fopen("temp_recovery.txt", "r");
            int recovered_val = 0;
            if(res) {
                if(fscanf(res, "%d", &recovered_val) == 1) {
                    val = LLVMConstInt(LLVMInt32Type(), recovered_val, 0);
                }
                fclose(res);
                remove("temp_recovery.txt"); 
            }
        }
    }

    if(val) {
        LLVMValueRef args[] = { fmt, val };
        LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
    } else {
        fprintf(stderr, " [Error] வேரியபிள் '%s' எங்கும் காணப்படவில்லை!\n", var_name);
    }
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

        fprintf(stderr, "[Codegen] Logic: %s = %s + %s completed.\n", res_name, var1, var2);
    }
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