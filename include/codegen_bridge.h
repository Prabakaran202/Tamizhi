#ifndef CODEGEN_BRIDGE_H
#define CODEGEN_BRIDGE_H

#include "codegen.h"
#include "ast.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Target.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Transforms/PassBuilder.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int call_depth = 0; 
int var_count = 0;


// =========================================================================
// 🌟 மாடர்ன் LLVM குளோபல் எக்ஸ்டெர்ன் மாறிகள்
// =========================================================================
extern LLVMContextRef context;
extern LLVMModuleRef module;
extern LLVMBuilderRef builder;
extern LLVMTargetMachineRef target_machine;
extern LLVMTypeRef printf_type;
extern LLVMValueRef printf_func;
extern LLVMValueRef current_function;

// லினக்ஸ் சிஸ்டம் கால்களுக்கான (இயக்கு / system_call) எல்எல்விஎம் டிராக்கர்ஸ்
extern LLVMTypeRef system_type;
extern LLVMValueRef system_func;

extern int loop_counter;
extern int if_counter;


// =========================================================================
// 🌟 நெஸ்டட் லூப் ஸ்டேக்
// =========================================================================
typedef struct {
    LLVMValueRef i_ptr;
    LLVMBasicBlockRef cond_block;
    LLVMBasicBlockRef body_block;
    LLVMBasicBlockRef after_block;
} LoopContext;

#define MAX_LOOPS 100
extern LoopContext loop_stack[MAX_LOOPS];
extern int loop_top;


// =========================================================================
// 🌟 இஃப்-எல்ஸ் ஸ்டேக்
// =========================================================================
#define MAX_IF_DEPTH 256
typedef struct {
    LLVMBasicBlockRef true_block;
    LLVMBasicBlockRef false_block;
    LLVMBasicBlockRef end_block;
    int has_else;
} IfContext;

extern IfContext if_stack[MAX_IF_DEPTH];
extern int if_top;

// =========================================================================
// 🌟 சிம்பல் மற்றும் ஃபங்ஷன் டேபிள்
// =========================================================================
#define MAX_VARS 100
#define MAX_FUNCS 50

typedef struct {
    char name[100];
    LLVMValueRef alloca_ptr;
    int is_str_type;
    int has_static_val;
    int static_val;
    int scope_depth;
} Variable;

extern Variable symbol_table[MAX_VARS];
extern int var_count;
extern int call_depth; 

typedef struct {
    char name[100];
    LLVMValueRef func_ref;
    LLVMBasicBlockRef resume_block;
} TamizhiFunction;

extern TamizhiFunction function_table[MAX_FUNCS];
extern int func_count;

// =========================================================================
// 🚀 தமிழியின் 26 மாடுலர் ஃபங்ஷன்களின் எக்ஸ்டெர்ன் டிக்ளரேஷன்ஸ்
// =========================================================================
void tamizhi_codegen_trim(char *str);
LLVMValueRef create_entry_alloca(LLVMValueRef function, LLVMTypeRef type, const char* name);
LLVMValueRef tamizhi_evaluate_ast(ASTNode* node);
void tamizhi_gen_math_ast(char* res_name, ASTNode* root);
void tamizhi_generate_universal_bitcode(const char* filename);
void tamizhi_binary_to_dna_storage(const char* filename);
void tamizhi_codegen_init(void);
void tamizhi_gen_function_start(char* func_name);
void tamizhi_gen_function_end(void);
void tamizhi_gen_function_call(char* func_name);
void tamizhi_generate_entry(void);
void tamizhi_gen_var(char* name, int value);
void tamizhi_gen_str(char* name, char* value);
void tamizhi_gen_math_op(char* res_name, char* v1, char* op, char* var2);
void tamizhi_gen_print(char* var_name);
void tamizhi_gen_ternary(char* res_name, char* v1, char* op, char* var2, char* true_val, char* false_val);

// [ADDED]: லினக்ஸ் சிஸ்டம் ஆட்டோமேஷன் ஷெல் இன்ஜின் டிக்ளரேஷன்
void tamizhi_gen_system_call(char* command);

void tamizhi_gen_loop_start(int limit);
void tamizhi_gen_loop_end(void);
void tamizhi_gen_if_start(char* lhs, char* rel_op, char* rhs);
void tamizhi_gen_if_body_end(void);
void tamizhi_gen_else_start(void);
void tamizhi_gen_if_end(void);
void tamizhi_gen_return(char* return_val);
void tamizhi_gen_assign_from_return(char* var_name);
void tamizhi_codegen_finish(void);   // ← இதை add பண்ணு
void tamizhi_codegen_destroy(void);



#endif
