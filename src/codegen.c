#include "codegen.h"
#include "ast.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Target.h>
#include <llvm-c/BitWriter.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern void encode_logic(const char* input_path, const char* output_path);
extern int current_line; 

LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMTargetMachineRef target_machine = NULL; 
LLVMTypeRef printf_type;
LLVMValueRef printf_func;

int loop_counter = 0;

typedef struct {
    LLVMValueRef i_ptr;
    LLVMBasicBlockRef cond_block;
    LLVMBasicBlockRef body_block;
    LLVMBasicBlockRef after_block;
} LoopContext;

LoopContext loop_stack[100];
int loop_top = -1;

typedef struct {
    char name[100]; 
    LLVMValueRef alloca_ptr;
    int is_str_type;
    int has_static_val;
    int static_val;
} Variable;

Variable symbol_table[100];
int var_count = 0;

void tamizhi_codegen_trim(char *str) {
    char *trim_p = str;
    while(isspace((unsigned char)*trim_p)) trim_p++;
    int len = strlen(trim_p);
    while(len > 0 && isspace((unsigned char)trim_p[len-1])) {
        trim_p[len-1] = '\0';
        len--;
    }
    memmove(str, trim_p, strlen(trim_p) + 1);
}

LLVMValueRef create_entry_alloca(LLVMValueRef function, LLVMTypeRef type, const char* name) {
    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
    LLVMBasicBlockRef entry_bb = LLVMGetEntryBasicBlock(function);
    LLVMValueRef first_inst = LLVMGetFirstInstruction(entry_bb);
    if (first_inst) LLVMPositionBuilderBefore(builder, first_inst);
    else LLVMPositionBuilderAtEnd(builder, entry_bb);
    LLVMValueRef alloca = LLVMBuildAlloca(builder, type, name);
    if (current_bb) LLVMPositionBuilderAtEnd(builder, current_bb);
    return alloca;
}

// 🌳 AST Tree Walker & Math Gen
LLVMValueRef tamizhi_evaluate_ast(ASTNode* node) {
    if (node == NULL) return NULL;
    if (node->type == AST_NUMBER) return LLVMConstInt(LLVMInt32Type(), node->data.num_value, 0);
    else if (node->type == AST_IDENTIFIER) {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, node->data.var_name) == 0)
                return LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "ast_load");
        }
    } else if (node->type == AST_BINARY_OP) {
        LLVMValueRef l = tamizhi_evaluate_ast(node->data.binop.left);
        LLVMValueRef r = tamizhi_evaluate_ast(node->data.binop.right);
        if (strcmp(node->data.binop.op, "+") == 0) return LLVMBuildAdd(builder, l, r, "add");
        if (strcmp(node->data.binop.op, "-") == 0) return LLVMBuildSub(builder, l, r, "sub");
        if (strcmp(node->data.binop.op, "*") == 0) return LLVMBuildMul(builder, l, r, "mul");
        if (strcmp(node->data.binop.op, "/") == 0) return LLVMBuildSDiv(builder, l, r, "div");
    }
    return NULL;
}

void tamizhi_gen_math_ast(char* res_name, ASTNode* root) {
    char clean_res[100]; snprintf(clean_res, sizeof(clean_res), "%s", res_name); tamizhi_codegen_trim(clean_res);
    LLVMValueRef math_res = tamizhi_evaluate_ast(root);
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, clean_res) == 0) {
            LLVMBuildStore(builder, math_res, symbol_table[i].alloca_ptr);
            return;
        }
    }
    LLVMValueRef alloca = create_entry_alloca(LLVMGetNamedFunction(module, "main"), LLVMInt32Type(), clean_res);
    LLVMBuildStore(builder, math_res, alloca);
    strcpy(symbol_table[var_count].name, clean_res); symbol_table[var_count].alloca_ptr = alloca; var_count++;
}

void tamizhi_gen_print(char* var_name) {
    LLVMValueRef val = NULL; int is_string = 0;
    char clean_name[1024]; snprintf(clean_name, sizeof(clean_name), "%s", var_name); tamizhi_codegen_trim(clean_name);
    if (clean_name[0] == '"') { val = LLVMBuildGlobalStringPtr(builder, clean_name+1, "str"); is_string = 1; }
    else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, clean_name) == 0) {
                if(symbol_table[i].is_str_type) { val = symbol_table[i].alloca_ptr; is_string = 1; }
                else val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "load");
                break;
            }
        }
    }
    if(!val && loop_top >= 0 && strcmp(clean_name, "i") == 0) val = LLVMBuildLoad2(builder, LLVMInt32Type(), loop_stack[loop_top].i_ptr, "load_loop");
    if(!val) val = LLVMConstInt(LLVMInt32Type(), atoi(clean_name), 0);
    const char* fmt = is_string ? "%s\n" : "%d\n";
    LLVMValueRef args[] = { LLVMBuildGlobalStringPtr(builder, fmt, "fmt"), val };
    LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
}

void tamizhi_gen_ternary(char* r, char* v1, char* op, char* v2, char* t, char* f) {
    LLVMValueRef val1 = LLVMConstInt(LLVMInt32Type(), atoi(v1), 0), val2 = LLVMConstInt(LLVMInt32Type(), atoi(v2), 0);
    LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntSGT, val1, val2, "ternary_cond");
    LLVMValueRef res = LLVMBuildSelect(builder, cond, LLVMConstInt(LLVMInt32Type(), atoi(t), 0), LLVMConstInt(LLVMInt32Type(), atoi(f), 0), "sel");
    LLVMValueRef alloca = create_entry_alloca(LLVMGetNamedFunction(module, "main"), LLVMInt32Type(), r);
    LLVMBuildStore(builder, res, alloca);
    strcpy(symbol_table[var_count].name, r); symbol_table[var_count].alloca_ptr = alloca; var_count++;
}

void tamizhi_gen_loop_start(int limit) {
    LLVMValueRef func = LLVMGetNamedFunction(module, "main");
    loop_top++;
    LoopContext* ctx = &loop_stack[loop_top];
    ctx->cond_block = LLVMAppendBasicBlock(func, "cond"); ctx->body_block = LLVMAppendBasicBlock(func, "body");
    ctx->after_block = LLVMAppendBasicBlock(func, "after");
    ctx->i_ptr = create_entry_alloca(func, LLVMInt32Type(), "i");
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), 0, 0), ctx->i_ptr);
    LLVMBuildBr(builder, ctx->cond_block);
    LLVMPositionBuilderAtEnd(builder, ctx->cond_block);
    LLVMBuildCondBr(builder, LLVMBuildICmp(builder, LLVMIntSLT, LLVMBuildLoad2(builder, LLVMInt32Type(), ctx->i_ptr, "i_val"), LLVMConstInt(LLVMInt32Type(), limit, 0), "cond"), ctx->body_block, ctx->after_block);
    LLVMPositionBuilderAtEnd(builder, ctx->body_block);
}

void tamizhi_gen_loop_end() {
    LoopContext* ctx = &loop_stack[loop_top];
    LLVMBuildStore(builder, LLVMBuildAdd(builder, LLVMBuildLoad2(builder, LLVMInt32Type(), ctx->i_ptr, "i_val"), LLVMConstInt(LLVMInt32Type(), 1, 0), "next_i"), ctx->i_ptr);
    LLVMBuildBr(builder, ctx->cond_block); 
    LLVMPositionBuilderAtEnd(builder, ctx->after_block); loop_top--;
}

void tamizhi_codegen_init() {
    LLVMInitializeAllTargets(); module = LLVMModuleCreateWithName("tamizhi"); builder = LLVMCreateBuilder();
    printf_func = LLVMAddFunction(module, "printf", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1));
}

void tamizhi_generate_entry() { if(!LLVMGetNamedFunction(module, "main")) LLVMAppendBasicBlock(LLVMAddFunction(module, "main", LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0)), "entry"); }
void tamizhi_gen_var(char* n, int v) { 
    LLVMValueRef alloca = create_entry_alloca(LLVMGetNamedFunction(module, "main"), LLVMInt32Type(), n);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), v, 0), alloca);
    strcpy(symbol_table[var_count].name, n); symbol_table[var_count].alloca_ptr = alloca; var_count++;
}
void tamizhi_gen_str(char* n, char* v) { 
    symbol_table[var_count].alloca_ptr = LLVMBuildGlobalStringPtr(builder, v, "str");
    strcpy(symbol_table[var_count].name, n); symbol_table[var_count].is_str_type = 1; var_count++;
}
void tamizhi_codegen_finish() { 
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    LLVMWriteBitcodeToFile(module, "output.bc"); system("lli output.bc");
}
