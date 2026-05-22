#include "codegen.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Target.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Transforms/PassManagerBuilder.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/Transforms/Utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>

/* =====================================================================
 * 1. COMPILER ARCHITECTURE CONFIGURATIONS & STRUCTURES
 * ===================================================================== */

#define TAMIZHI_MAX_SYMBOLS 1024
#define TAMIZHI_MAX_LOOPS   128
#define TAMIZHI_MAX_ARGS    16

typedef enum {
    TY_INT32,
    TY_STRING,
    TY_VOID
} TamizhiType;

typedef struct {
    char name[128];
    LLVMValueRef alloca_ptr;
    TamizhiType type;
    int has_static_val;
    int static_val;
} TamizhiVariable;

typedef struct {
    LLVMValueRef i_ptr;
    LLVMBasicBlockRef cond_block;
    LLVMBasicBlockRef body_block;
    LLVMBasicBlockRef after_block;
    int loop_id;
} TamizhiLoopContext;

typedef struct {
    char name[128];
    LLVMValueRef func_ref;
    TamizhiType return_type;
    TamizhiType arg_types[TAMIZHI_MAX_ARGS];
    int arg_count;
} TamizhiFunction;

/* Global Backend States */
LLVMModuleRef module = NULL;
LLVMBuilderRef builder = NULL;
LLVMTargetMachineRef target_machine = NULL;
LLVMTypeRef printf_type = NULL;
LLVMValueRef printf_func = NULL;

static TamizhiVariable symbol_table[TAMIZHI_MAX_SYMBOLS];
static int var_count = 0;

static TamizhiLoopContext loop_stack[TAMIZHI_MAX_LOOPS];
static int loop_top = -1;
int loop_counter = 0;

static TamizhiFunction function_table[128];
static int func_count = 0;

/* Forward Declarations of Core Utilities */
void tamizhi_codegen_trim(char *str);
LLVMValueRef create_entry_alloca(LLVMValueRef function, LLVMTypeRef type, const char* name);
static int tamizhi_lookup_variable(const char *name);
static int tamizhi_sanitize_identifier(const char *name, char *out_name, size_t max_len);

/* =====================================================================
 * 2. SYMBOL TABLE & UTILITY ENGINE
 * ===================================================================== */

void tamizhi_codegen_trim(char *str) {
    if (!str) return;
    unsigned char *trim_p = (unsigned char *)str;
    while (isspace(*trim_p)) trim_p++;
    
    size_t len = strlen((char *)trim_p);
    while (len > 0 && (isspace(trim_p[len - 1]) || trim_p[len - 1] == ';')) {
        trim_p[len - 1] = '\0';
        len--;
    }
    memmove(str, trim_p, len + 1);
}

static int tamizhi_sanitize_identifier(const char *name, char *out_name, size_t max_len) {
    if (!name || strlen(name) == 0 || max_len == 0) return 0;
    size_t i = 0, j = 0;
    
    /* Ensure deterministic naming format stripping unsafe characters */
    for (i = 0; name[i] != '\0' && j < (max_len - 1); i++) {
        if (isalnum((unsigned char)name[i]) || name[i] == '_' || (unsigned char)name[i] > 127) {
            out_name[j++] = name[i];
        }
    }
    out_name[j] = '\0';
    return (j > 0);
}

static int tamizhi_lookup_variable(const char *name) {
    char clean_name[128];
    if (!tamizhi_sanitize_identifier(name, clean_name, sizeof(clean_name))) return -1;
    
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, clean_name) == 0) {
            return i;
        }
    }
    return -1;
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

    if (current_bb) {
        LLVMPositionBuilderAtEnd(builder, current_bb);
    } else {
        LLVMPositionBuilderAtEnd(builder, entry_bb);
    }

    return alloca;
}

/* =====================================================================
 * 3. CODEGEN CORE LIFECYCLE MANAGEMENT
 * ===================================================================== */

void tamizhi_codegen_init(void) {
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllAsmPrinters();

    module = LLVMModuleCreateWithName("tamizhi_core_engine");
    builder = LLVMCreateBuilder();

    char *target_triple = NULL;
#ifdef __ANDROID__
    target_triple = strdup("aarch64-unknown-linux-android30");
#else
    target_triple = LLVMGetDefaultTargetTriple();
#endif

    LLVMSetTarget(module, target_triple);

    char *error = NULL;
    LLVMTargetRef target;
    if (LLVMGetTargetFromTriple(target_triple, &target, &error)) {
        fprintf(stderr, "[Fatal Codegen Error] Initialization Failed: %s\n", error);
        LLVMDisposeMessage(error);
#ifdef __ANDROID__
        free(target_triple);
#endif
        exit(1);
    }

    target_machine = LLVMCreateTargetMachine(target, target_triple, "generic", "", 
                                             LLVMCodeGenLevelDefault, LLVMRelocDefault, 
                                             LLVMCodeModelDefault);

    LLVMTargetDataRef data_layout = LLVMCreateTargetDataLayout(target_machine);
    LLVMSetModuleDataLayout(module, data_layout);
    LLVMDisposeTargetData(data_layout);

    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    printf_type = LLVMFunctionType(LLVMInt32Type(), printf_args, 1, 1);
    printf_func = LLVMAddFunction(module, "printf", printf_type);

    fprintf(stderr, " [Codegen Engine] Initialized Target Context Architecture Triple: %s\n", target_triple);

#ifdef __ANDROID__
    free(target_triple);
#else
    LLVMDisposeMessage(target_triple);
#endif
}

void tamizhi_generate_entry(void) {
    if (LLVMGetNamedFunction(module, "main")) return; 
    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    
    /* Auto register main function metadata in layout */
    snprintf(function_table[func_count].name, 128, "main");
    function_table[func_count].func_ref = main_func;
    function_table[func_count].return_type = TY_INT32;
    function_table[func_count].arg_count = 0;
    func_count++;
}

/* =====================================================================
 * 4. VARIABLE & DATA EXPRESSION EMISSION
 * ===================================================================== */

void tamizhi_gen_var(char* name, int value) {
    char clean_name[128];
    if (!tamizhi_sanitize_identifier(name, clean_name, sizeof(clean_name))) return;

    LLVMValueRef func = LLVMGetNamedFunction(module, "main");
    if (!func) return;

    int existing_idx = tamizhi_lookup_variable(clean_name);
    if (existing_idx != -1) {
        symbol_table[existing_idx].static_val = value;
        symbol_table[existing_idx].has_static_val = 1;
        symbol_table[existing_idx].type = TY_INT32;
        if (symbol_table[existing_idx].alloca_ptr) {
            LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), symbol_table[existing_idx].alloca_ptr);
        }
        return;
    }

    if (var_count >= TAMIZHI_MAX_SYMBOLS) {
        fprintf(stderr, "[Internal Backend Error] Symbol Table Limit Exhausted!\n");
        return;
    }

    LLVMValueRef alloca = create_entry_alloca(func, LLVMInt32Type(), clean_name);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), alloca);

    snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_name);
    symbol_table[var_count].alloca_ptr = alloca;
    symbol_table[var_count].type = TY_INT32;
    symbol_table[var_count].static_val = value;
    symbol_table[var_count].has_static_val = 1;
    var_count++;
}

void tamizhi_gen_str(char* name, char* value) {
    char clean_name[128];
    if (!tamizhi_sanitize_identifier(name, clean_name, sizeof(clean_name))) return;

    int existing_idx = tamizhi_lookup_variable(clean_name);
    if (existing_idx != -1) {
        LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(builder, value, "str_lit");
        symbol_table[existing_idx].alloca_ptr = str_ptr;
        symbol_table[existing_idx].type = TY_STRING;
        symbol_table[existing_idx].has_static_val = 0;
        return;
    }

    if (var_count >= TAMIZHI_MAX_SYMBOLS) return;

    LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(builder, value, "str_lit");
    snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_name);
    symbol_table[var_count].alloca_ptr = str_ptr; 
    symbol_table[var_count].type = TY_STRING;
    symbol_table[var_count].has_static_val = 0;
    var_count++;
}

/* =====================================================================
 * 5. MATHEMATICAL & TERNARY INSTRUCTION PIPELINES
 * ===================================================================== */

void tamizhi_gen_math_op(char* res_name, char* var1, char* op, char* var2) {
    char clean_res[128], clean_v1[128], clean_v2[128];
    if (!tamizhi_sanitize_identifier(res_name, clean_res, sizeof(clean_res))) return;
    
    snprintf(clean_v1, sizeof(clean_v1), "%s", var1); tamizhi_codegen_trim(clean_v1);
    snprintf(clean_v2, sizeof(clean_v2), "%s", var2); tamizhi_codegen_trim(clean_v2);

    LLVMValueRef v1_val = NULL, v2_val = NULL;
    int s_val1 = 0, s_val2 = 0;
    int f1 = 0, f2 = 0;

    /* Left-hand operand valuation */
    if (isdigit((unsigned char)clean_v1[0]) || (clean_v1[0] == '-' && isdigit((unsigned char)clean_v1[1]))) {
        v1_val = LLVMConstInt(LLVMInt32Type(), atoi(clean_v1), 0);
        s_val1 = atoi(clean_v1);
        f1 = 1;
    } else {
        int idx = tamizhi_lookup_variable(clean_v1);
        if (idx != -1) {
            if (symbol_table[idx].has_static_val) {
                s_val1 = symbol_table[idx].static_val;
                f1 = 1;
            }
            v1_val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[idx].alloca_ptr, "v1_load");
        }
    }

    /* Right-hand operand valuation */
    if (isdigit((unsigned char)clean_v2[0]) || (clean_v2[0] == '-' && isdigit((unsigned char)clean_v2[1]))) {
        v2_val = LLVMConstInt(LLVMInt32Type(), atoi(clean_v2), 0);
        s_val2 = atoi(clean_v2);
        f2 = 1;
    } else {
        int idx = tamizhi_lookup_variable(clean_v2);
        if (idx != -1) {
            if (symbol_table[idx].has_static_val) {
                s_val2 = symbol_table[idx].static_val;
                f2 = 1;
            }
            v2_val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[idx].alloca_ptr, "v2_load");
        }
    }

    if (!v1_val || !v2_val) {
        fprintf(stderr, "[Semantic Error] Missing operand types for operation: %s\n", clean_res);
        return;
    }

    LLVMValueRef math_res = NULL;
    int calculated_val = 0;

    if (strcmp(op, "+") == 0) {
        math_res = LLVMBuildAdd(builder, v1_val, v2_val, "add_tmp");
        calculated_val = s_val1 + s_val2;
    } else if (strcmp(op, "-") == 0) {
        math_res = LLVMBuildSub(builder, v1_val, v2_val, "sub_tmp");
        calculated_val = s_val1 - s_val2;
    } else if (strcmp(op, "*") == 0) {
        math_res = LLVMBuildMul(builder, v1_val, v2_val, "mul_tmp");
        calculated_val = s_val1 * s_val2;
    } else if (strcmp(op, "/") == 0) {
        if (f2 && s_val2 == 0) {
            fprintf(stderr, "[Fatal Runtime Exception] Compile-time isolated Division by zero on mapping: %s\n", clean_res);
            return;
        }
        math_res = LLVMBuildSDiv(builder, v1_val, v2_val, "div_tmp");
        if (s_val2 != 0) calculated_val = s_val1 / s_val2;
    }

    if (math_res) {
        LLVMValueRef target_ptr = NULL;
        int found_idx = tamizhi_lookup_variable(clean_res);

        if (found_idx == -1 && var_count < TAMIZHI_MAX_SYMBOLS) {
            LLVMValueRef func = LLVMGetNamedFunction(module, "main");
            target_ptr = create_entry_alloca(func, LLVMInt32Type(), clean_res);
            snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
            symbol_table[var_count].alloca_ptr = target_ptr;
            symbol_table[var_count].type = TY_INT32;
            found_idx = var_count;
            var_count++;
        } else if (found_idx != -1) {
            target_ptr = symbol_table[found_idx].alloca_ptr;
        }

        if (target_ptr) {
            LLVMBuildStore(builder, math_res, target_ptr);
            if (f1 && f2) {
                symbol_table[found_idx].static_val = calculated_val;
                symbol_table[found_idx].has_static_val = 1;
            } else {
                symbol_table[found_idx].has_static_val = 0;
            }
        }
    }
}

void tamizhi_gen_ternary(char* res_name, char* v1, char* op, char* v2, char* true_val, char* false_val) {
    char clean_res[128], clean_v1[128], clean_v2[128], clean_t[128], clean_f[128];
    if (!tamizhi_sanitize_identifier(res_name, clean_res, sizeof(clean_res))) return;
    
    snprintf(clean_v1, sizeof(clean_v1), "%s", v1); tamizhi_codegen_trim(clean_v1);
    snprintf(clean_v2, sizeof(clean_v2), "%s", v2); tamizhi_codegen_trim(clean_v2);
    snprintf(clean_t, sizeof(clean_t), "%s", true_val); tamizhi_codegen_trim(clean_t);
    snprintf(clean_f, sizeof(clean_f), "%s", false_val); tamizhi_codegen_trim(clean_f);
    
    char *semi = strchr(clean_f, ';');
    if (semi) *semi = '\0';
    tamizhi_codegen_trim(clean_f);

    LLVMValueRef val1 = NULL, val2 = NULL;

    if (isdigit((unsigned char)clean_v1[0]) || (clean_v1[0] == '-' && isdigit((unsigned char)clean_v1[1]))) {
        val1 = LLVMConstInt(LLVMInt32Type(), atoi(clean_v1), 0);
    } else {
        int idx = tamizhi_lookup_variable(clean_v1);
        if (idx != -1) val1 = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[idx].alloca_ptr, "t_v1");
    }

    if (isdigit((unsigned char)clean_v2[0]) || (clean_v2[0] == '-' && isdigit((unsigned char)clean_v2[1]))) {
        val2 = LLVMConstInt(LLVMInt32Type(), atoi(clean_v2), 0);
    } else {
        int idx = tamizhi_lookup_variable(clean_v2);
        if (idx != -1) val2 = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[idx].alloca_ptr, "t_v2");
    }

    if (!val1) val1 = LLVMConstInt(LLVMInt32Type(), 0, 0);
    if (!val2) val2 = LLVMConstInt(LLVMInt32Type(), 0, 0);

    LLVMIntPredicate pred = LLVMIntEQ;
    if (strcmp(op, "<") == 0) pred = LLVMIntSLT;
    else if (strcmp(op, ">") == 0) pred = LLVMIntSGT;
    else if (strcmp(op, "==") == 0) pred = LLVMIntEQ;
    else if (strcmp(op, "!=") == 0) pred = LLVMIntNE;
    else if (strcmp(op, "<=") == 0) pred = LLVMIntSLE;
    else if (strcmp(op, ">=") == 0) pred = LLVMIntSGE;

    LLVMValueRef cond = LLVMBuildICmp(builder, pred, val1, val2, "tern_cmp");

    LLVMValueRef t_val = NULL, f_val = NULL;
    if (isdigit((unsigned char)clean_t[0]) || (clean_t[0] == '-' && isdigit((unsigned char)clean_t[1]))) {
        t_val = LLVMConstInt(LLVMInt32Type(), atoi(clean_t), 0);
    } else {
        int idx = tamizhi_lookup_variable(clean_t);
        if (idx != -1) t_val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[idx].alloca_ptr, "t_val_ld");
    }

    if (isdigit((unsigned char)clean_f[0]) || (clean_f[0] == '-' && isdigit((unsigned char)clean_f[1]))) {
        f_val = LLVMConstInt(LLVMInt32Type(), atoi(clean_f), 0);
    } else {
        int idx = tamizhi_lookup_variable(clean_f);
        if (idx != -1) f_val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[idx].alloca_ptr, "f_val_ld");
    }

    if (!t_val) t_val = LLVMConstInt(LLVMInt32Type(), 0, 0);
    if (!f_val) f_val = LLVMConstInt(LLVMInt32Type(), 0, 0);

    LLVMValueRef select_res = LLVMBuildSelect(builder, cond, t_val, f_val, "ternary_sugar");

    LLVMValueRef target_ptr = NULL;
    int target_idx = tamizhi_lookup_variable(clean_res);
    if (target_idx == -1 && var_count < TAMIZHI_MAX_SYMBOLS) {
        LLVMValueRef func = LLVMGetNamedFunction(module, "main");
        target_ptr = create_entry_alloca(func, LLVMInt32Type(), clean_res);
        snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
        symbol_table[var_count].alloca_ptr = target_ptr;
        symbol_table[var_count].type = TY_INT32;
        target_idx = var_count;
        var_count++;
    } else if (target_idx != -1) {
        target_ptr = symbol_table[target_idx].alloca_ptr;
    }

    if (target_ptr) {
        LLVMBuildStore(builder, select_res, target_ptr);
        symbol_table[target_idx].has_static_val = 0; /* Dynamic select breaks pure static tracking */
    }
}

/* =====================================================================
 * 6. CFG LOOP STACK EMISSION ENGINE
 * ===================================================================== */

void tamizhi_gen_loop_start(int limit) {
    LLVMValueRef func = LLVMGetNamedFunction(module, "main");
    if (!func) return;

    if (loop_top >= (TAMIZHI_MAX_LOOPS - 1)) {
        fprintf(stderr, "[Fatal Code Generation Overflow] Nested loop limits breached.\n");
        return;
    }

    loop_top++;
    TamizhiLoopContext* ctx = &loop_stack[loop_top];
    ctx->loop_id = loop_counter;

    char cond_name[32], body_name[32], after_name[32];
    snprintf(cond_name, sizeof(cond_name), "loop_cond_%d", loop_counter);
    snprintf(body_name, sizeof(body_name), "loop_body_%d", loop_counter);
    snprintf(after_name, sizeof(after_name), "loop_after_%d", loop_counter);
    loop_counter++;

    ctx->cond_block = LLVMAppendBasicBlock(func, cond_name);
    ctx->body_block = LLVMAppendBasicBlock(func, body_name);
    ctx->after_block = LLVMAppendBasicBlock(func, after_name);

    /* Context Alloc for explicit nested counter values */
    ctx->i_ptr = create_entry_alloca(func, LLVMInt32Type(), "tamizhi_loop_i");
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), 0, 0), ctx->i_ptr);
    LLVMBuildBr(builder, ctx->cond_block);

    LLVMPositionBuilderAtEnd(builder, ctx->cond_block);
    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), ctx->i_ptr, "i_eval");
    LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntSLT, i_val, LLVMConstInt(LLVMInt32Type(), limit, 0), "cmp_cond");
    LLVMBuildCondBr(builder, cond, ctx->body_block, ctx->after_block);

    LLVMPositionBuilderAtEnd(builder, ctx->body_block);
}

void tamizhi_gen_loop_end(void) {
    if (loop_top < 0) {
        fprintf(stderr, "[Control Flow Malformed Exception] Attempted loop stack closure on empty vector\n");
        return;
    }
    TamizhiLoopContext* ctx = &loop_stack[loop_top];

    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), ctx->i_ptr, "i_inc_ld");
    LLVMValueRef next_val = LLVMBuildAdd(builder, i_val, LLVMConstInt(LLVMInt32Type(), 1, 0), "i_step");
    LLVMBuildStore(builder, next_val, ctx->i_ptr);

    LLVMBuildBr(builder, ctx->cond_block); 
    LLVMPositionBuilderAtEnd(builder, ctx->after_block);
    loop_top--;
}

void tamizhi_gen_print(char* var_name) {
    LLVMValueRef val = NULL;
    int is_string = 0;
    char clean_name[1024];
    snprintf(clean_name, sizeof(clean_name), "%s", var_name);
    tamizhi_codegen_trim(clean_name);

    int is_literal = 0;
    size_t len = strlen(clean_name);
    if ((clean_name[0] == '"' || clean_name[0] == '\'') && len > 2) {
        char temp[1024];
        strncpy(temp, clean_name + 1, len - 2);
        temp[len - 2] = '\0';
        strcpy(clean_name, temp);
        is_literal = 1;
    }

    if (!is_literal) {
        int idx = tamizhi_lookup_variable(clean_name);
        if (idx != -1