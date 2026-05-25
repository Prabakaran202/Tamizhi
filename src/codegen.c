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
    if (first_inst) {
        LLVMPositionBuilderBefore(builder, first_inst);
    } else {
        LLVMPositionBuilderAtEnd(builder, entry_bb);
    }
    LLVMValueRef alloca = LLVMBuildAlloca(builder, type, name);
    if (current_bb) LLVMPositionBuilderAtEnd(builder, current_bb);
    return alloca;
}

// 🌳 AST Tree Walker
LLVMValueRef tamizhi_evaluate_ast(ASTNode* node) {
    if (node == NULL) return NULL;
    if (node->type == AST_NUMBER) {
        return LLVMConstInt(LLVMInt32Type(), node->data.num_value, 0);
    }
    else if (node->type == AST_IDENTIFIER) {
        char clean_name[256];
        snprintf(clean_name, sizeof(clean_name), "%s", node->data.var_name);
        tamizhi_codegen_trim(clean_name);
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_name) == 0) {
                return LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "ast_load");
            }
        }
        return LLVMConstInt(LLVMInt32Type(), 0, 0);
    }
    else if (node->type == AST_BINARY_OP) {
        LLVMValueRef left_val = tamizhi_evaluate_ast(node->data.binop.left);
        LLVMValueRef right_val = tamizhi_evaluate_ast(node->data.binop.right);
        if (!left_val || !right_val) return NULL;
        if (strcmp(node->data.binop.op, "+") == 0) return LLVMBuildAdd(builder, left_val, right_val, "ast_add");
        if (strcmp(node->data.binop.op, "-") == 0) return LLVMBuildSub(builder, left_val, right_val, "ast_sub");
        if (strcmp(node->data.binop.op, "*") == 0) return LLVMBuildMul(builder, left_val, right_val, "ast_mul");
        if (strcmp(node->data.binop.op, "/") == 0) return LLVMBuildSDiv(builder, left_val, right_val, "ast_div");
    }
    return NULL;
}

void tamizhi_gen_math_ast(char* res_name, ASTNode* root) {
    char clean_res[100];
    snprintf(clean_res, sizeof(clean_res), "%s", res_name);
    tamizhi_codegen_trim(clean_res);
    LLVMValueRef math_res = tamizhi_evaluate_ast(root);
    if (!math_res) return;

    LLVMValueRef target_ptr = NULL;
    int found_idx = -1;
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, clean_res) == 0) {
            target_ptr = symbol_table[i].alloca_ptr;
            found_idx = i;
            break;
        }
    }
    if (!target_ptr && var_count < 100) {
        LLVMValueRef func = LLVMGetNamedFunction(module, "main");
        target_ptr = create_entry_alloca(func, LLVMInt32Type(), clean_res);
        snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
        symbol_table[var_count].alloca_ptr = target_ptr;
        symbol_table[var_count].is_str_type = 0;
        found_idx = var_count;
        var_count++;
    }
    if (target_ptr && found_idx != -1) {
        LLVMBuildStore(builder, math_res, target_ptr);
    }
}

// 🌟 UPDATED PRINT FUNCTION
void tamizhi_gen_print(char* var_name) {
    LLVMValueRef val = NULL;
    int is_string = 0;
    char clean_name[1024];
    snprintf(clean_name, sizeof(clean_name), "%s", var_name);
    tamizhi_codegen_trim(clean_name);

    int is_literal = 0;
    if ((clean_name[0] == '"' || clean_name[0] == '\'') && strlen(clean_name) > 2) {
        char temp[1024];
        strncpy(temp, clean_name + 1, strlen(clean_name) - 2);
        temp[strlen(clean_name) - 2] = '\0';
        strcpy(clean_name, temp);
        is_literal = 1;
    }

    if (!is_literal) {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, clean_name) == 0) {
                val = symbol_table[i].alloca_ptr;
                if (symbol_table[i].is_str_type) {
                    is_string = 1; 
                } else {
                    val = LLVMBuildLoad2(builder, LLVMInt32Type(), val, "load_ast_val");
                }
                break;
            }
        }
    }
    if(!val && loop_top >= 0 && strcmp(clean_name, "i") == 0) val = LLVMBuildLoad2(builder, LLVMInt32Type(), loop_stack[loop_top].i_ptr, "load_loop_i");
    if(!val && (isdigit((unsigned char)clean_name[0]) || clean_name[0] == '-') && !is_literal) val = LLVMConstInt(LLVMInt32Type(), atoi(clean_name), 0);
    if(!val) { val = LLVMBuildGlobalStringPtr(builder, clean_name, "str_lit"); is_string = 1; }

    if(val) {
        const char* fmt_str = is_string ? "%s\n" : "%d\n";
        LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, fmt_str, "fmt");
        LLVMValueRef args[] = { fmt, val };
        LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
    }
}

void tamizhi_codegen_init() {
    LLVMInitializeAllTargetInfos(); LLVMInitializeAllTargets(); LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmParsers(); LLVMInitializeAllAsmPrinters();
    module = LLVMModuleCreateWithName("tamizhi_engine");
    builder = LLVMCreateBuilder();
    char *target_triple = LLVMGetDefaultTargetTriple();
    LLVMSetTarget(module, target_triple);
    LLVMTargetRef target; char *error = NULL;
    LLVMGetTargetFromTriple(target_triple, &target, &error);
    target_machine = LLVMCreateTargetMachine(target, target_triple, "generic", "", LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);
    LLVMSetModuleDataLayout(module, LLVMCreateTargetDataLayout(target_machine));
    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    printf_type = LLVMFunctionType(LLVMInt32Type(), printf_args, 1, 1);
    printf_func = LLVMAddFunction(module, "printf", printf_type);
}

void tamizhi_generate_entry() {
    if (LLVMGetNamedFunction(module, "main")) return; 
    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
}

void tamizhi_gen_var(char* name, int value) {
    char clean_res[100]; snprintf(clean_res, sizeof(clean_res), "%s", name); tamizhi_codegen_trim(clean_res);
    LLVMValueRef func = LLVMGetNamedFunction(module, "main");
    LLVMValueRef alloca = create_entry_alloca(func, LLVMInt32Type(), clean_res);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), alloca);
    snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
    symbol_table[var_count].alloca_ptr = alloca;
    symbol_table[var_count].is_str_type = 0;
    var_count++;
}

void tamizhi_gen_str(char* name, char* value) {
    char clean_res[100]; snprintf(clean_res, sizeof(clean_res), "%s", name); tamizhi_codegen_trim(clean_res);
    LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(builder, value, "str_lit");
    snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
    symbol_table[var_count].alloca_ptr = str_ptr; symbol_table[var_count].is_str_type = 1;
    var_count++;
}

void tamizhi_gen_loop_start(int limit) {
    LLVMValueRef func = LLVMGetNamedFunction(module, "main");
    loop_top++;
    LoopContext* ctx = &loop_stack[loop_top];
    ctx->cond_block = LLVMAppendBasicBlock(func, "cond");
    ctx->body_block = LLVMAppendBasicBlock(func, "body");
    ctx->after_block = LLVMAppendBasicBlock(func, "after");
    ctx->i_ptr = create_entry_alloca(func, LLVMInt32Type(), "i");
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), 0, 0), ctx->i_ptr);
    LLVMBuildBr(builder, ctx->cond_block);
    LLVMPositionBuilderAtEnd(builder, ctx->cond_block);
    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), ctx->i_ptr, "i_val");
    LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntSLT, i_val, LLVMConstInt(LLVMInt32Type(), limit, 0), "cond");
    LLVMBuildCondBr(builder, cond, ctx->body_block, ctx->after_block);
    LLVMPositionBuilderAtEnd(builder, ctx->body_block);
}

void tamizhi_gen_loop_end() {
    if(loop_top < 0) return;
    LoopContext* ctx = &loop_stack[loop_top];
    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), ctx->i_ptr, "i_val");
    LLVMBuildStore(builder, LLVMBuildAdd(builder, i_val, LLVMConstInt(LLVMInt32Type(), 1, 0), "next_i"), ctx->i_ptr);
    LLVMBuildBr(builder, ctx->cond_block); 
    LLVMPositionBuilderAtEnd(builder, ctx->after_block);
    loop_top--;
}

void tamizhi_codegen_finish() {
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    tamizhi_generate_universal_bitcode("output.bc");
    system("lli output.bc");
    LLVMDisposeBuilder(builder); LLVMDisposeModule(module);
}
