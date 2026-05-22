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

extern void encode_logic(const char* input_path, const char* output_path);

LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMTargetMachineRef target_machine = NULL; 
LLVMTypeRef printf_type;
LLVMValueRef printf_func;

int loop_counter = 0;

// ==========================================
// 🌟 நெஸ்டட் லூப் ஆதரவிற்கான ஸ்டேக் கட்டமைப்பு
// ==========================================
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

typedef struct {
    char name[100];
    LLVMValueRef func_ref;
} TamizhiFunction;

TamizhiFunction function_table[50];
int func_count = 0;

// 🌟 கோடெஜன் லெவலில் பெயர்களை சுத்தமாக்கும் பக்கா ட்ரிம் மெக்கானிசம்
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

// ======================================================================
// 🌟 ஆண்ட்ராய்டு ARM64 பாயிண்டர் சிதைவு இல்லாத தற்காலிக ரோல்பேக் அலோகேஷன்
// ======================================================================
LLVMValueRef create_entry_alloca(LLVMValueRef function, LLVMTypeRef type, const char* name) {
    // மெயின் பில்டர் இப்போ இருக்குற தற்போதைய இன்செர்ட் பிளாக்கை லாக் செய்து வைப்போம்
    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
    
    // மெயின் ஃபங்ஷனோட என்ட்ரி பிளாக்கின் தொடக்கத்திற்கு பில்டரை தற்காலிகமாக கொண்டு செல்வோம்
    LLVMBasicBlockRef entry_bb = LLVMGetEntryBasicBlock(function);
    LLVMValueRef first_inst = LLVMGetFirstInstruction(entry_bb);
    
    if (first_inst) {
        LLVMPositionBuilderBefore(builder, first_inst);
    } else {
        LLVMPositionBuilderAtEnd(builder, entry_bb);
    }
    
    // மெயின் பில்டரைக் கொண்டே பாதுகாப்பாக அலோகேட் செய்கிறோம் (No temp_builder crash)
    LLVMValueRef alloca = LLVMBuildAlloca(builder, type, name);
    
    // அலோகேட் செய்து முடித்தவுடன் பில்டரை பழைய பொசிஷனுக்கே கச்சிதமாக ரோல்பேக் செய்கிறோம்!
    if (current_bb) {
        LLVMPositionBuilderAtEnd(builder, current_bb);
    }
    
    return alloca;
}

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

void tamizhi_codegen_init() {
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllAsmPrinters();

    module = LLVMModuleCreateWithName("tamizhi_engine");
    builder = LLVMCreateBuilder();

    // 🌟 பிரபாவின் மேஜிக்கலான மெமரி-சேஃப் டார்கெட் ட்ரிப்பிள் மேனேஜ்மென்ட் ஃபிக்ஸ்!
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
        fprintf(stderr, " [Codegen Error] %s\n", error);
        LLVMDisposeMessage(error);
        return;
    }

    target_machine = LLVMCreateTargetMachine(target, target_triple, "generic", "", 
                                             LLVMCodeGenLevelDefault, LLVMRelocDefault, 
                                             LLVMCodeModelDefault);

    LLVMSetModuleDataLayout(module, LLVMCreateTargetDataLayout(target_machine));

    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    printf_type = LLVMFunctionType(LLVMInt32Type(), printf_args, 1, 1);
    printf_func = LLVMAddFunction(module, "printf", printf_type);

    fprintf(stderr, " [Codegen] LLVM Engine Initialized with Target: %s\n", target_triple);

    // 🌟 பிரபாவின் கண்டிஷனல் மெமரி டிஸ்போஸ் ஃபிக்ஸ்
    #ifdef __ANDROID__
    free(target_triple);
    #else
    LLVMDisposeMessage(target_triple);
    #endif
}

void tamizhi_generate_entry() {
    if (LLVMGetNamedFunction(module, "main")) return; 
    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
}

void tamizhi_gen_var(char* name, int value) {
    char clean_res[100];
    snprintf(clean_res, sizeof(clean_res), "%s", name);
    tamizhi_codegen_trim(clean_res);

    LLVMValueRef func = LLVMGetNamedFunction(module, "main");

    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, clean_res) == 0) {
            symbol_table[i].static_val = value;
            symbol_table[i].has_static_val = 1;
            symbol_table[i].is_str_type = 0;
            if(symbol_table[i].alloca_ptr) {
                LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), symbol_table[i].alloca_ptr);
            }
            return;
        }
    }
    if (var_count >= 100) return;

    LLVMValueRef alloca = create_entry_alloca(func, LLVMInt32Type(), clean_res);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), alloca);

    snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
    symbol_table[var_count].alloca_ptr = alloca;
    symbol_table[var_count].is_str_type = 0;
    symbol_table[var_count].static_val = value;
    symbol_table[var_count].has_static_val = 1;
    var_count++;
}

void tamizhi_gen_str(char* name, char* value) {
    char clean_res[100];
    snprintf(clean_res, sizeof(clean_res), "%s", name);
    tamizhi_codegen_trim(clean_res);

    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, clean_res) == 0) {
            LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(builder, value, "str_lit");
            symbol_table[i].alloca_ptr = str_ptr;
            symbol_table[i].is_str_type = 1;
            symbol_table[i].has_static_val = 0;
            return;
        }
    }
    if (var_count >= 100) return;
    LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(builder, value, "str_lit");
    snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
    symbol_table[var_count].alloca_ptr = str_ptr; 
    symbol_table[var_count].is_str_type = 1;
    symbol_table[var_count].has_static_val = 0;
    var_count++;
}

void tamizhi_gen_math_op(char* res_name, char* var1, char* op, char* var2) {
    char clean_res[100], clean_v1[100], clean_v2[100];
    snprintf(clean_res, sizeof(clean_res), "%s", res_name); tamizhi_codegen_trim(clean_res);
    snprintf(clean_v1, sizeof(clean_v1), "%s", var1); tamizhi_codegen_trim(clean_v1);
    snprintf(clean_v2, sizeof(clean_v2), "%s", var2); tamizhi_codegen_trim(clean_v2);

    LLVMValueRef v1_val = NULL, v2_val = NULL;
    int s_val1 = 0, s_val2 = 0;
    int f1 = 0, f2 = 0;

    if(isdigit(clean_v1[0]) || clean_v1[0] == '-') {
        v1_val = LLVMConstInt(LLVMInt32Type(), atoi(clean_v1), 0);
        s_val1 = atoi(clean_v1);
        f1 = 1;
    } else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, clean_v1) == 0) {
                if(symbol_table[i].has_static_val) {
                    s_val1 = symbol_table[i].static_val;
                    f1 = 1;
                }
                v1_val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "v1");
                break;
            }
        }
    }

    if(isdigit(clean_v2[0]) || clean_v2[0] == '-') {
        v2_val = LLVMConstInt(LLVMInt32Type(), atoi(clean_v2), 0);
        s_val2 = atoi(clean_v2);
        f2 = 1;
    } else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, clean_v2) == 0) {
                if(symbol_table[i].has_static_val) {
                    s_val2 = symbol_table[i].static_val;
                    f2 = 1;
                }
                v2_val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "v2");
                break;
            }
        }
    }

    if(v1_val && v2_val) {
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
            // 🌟 ஜீரோ வகுத்தல் பாதுகாப்பு வளையம் (Safe Division)
            if(f2 && s_val2 == 0) {
                fprintf(stderr, "[Runtime Error] Division by zero!\n");
                return;
            }
            math_res = LLVMBuildSDiv(builder, v1_val, v2_val, "div_tmp");
            if (s_val2 != 0) calculated_val = s_val1 / s_val2;
        }

        if (math_res) {
            LLVMValueRef target_ptr = NULL;
            int found_idx = -1;

            for(int i = 0; i < var_count; i++) {
                if(strcmp(symbol_table[i].name, clean_res) == 0) {
                    target_ptr = symbol_table[i].alloca_ptr;
                    found_idx = i;
                    break;
                }
            }

            if(!target_ptr && var_count < 100) {
                LLVMValueRef func = LLVMGetNamedFunction(module, "main");
                target_ptr = create_entry_alloca(func, LLVMInt32Type(), clean_res);
                snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
                symbol_table[var_count].alloca_ptr = target_ptr;
                symbol_table[var_count].is_str_type = 0;
                found_idx = var_count;
                var_count++;
            }

            if(target_ptr && found_idx != -1) {
                LLVMBuildStore(builder, math_res, target_ptr);
                if(f1 && f2) {
                    symbol_table[found_idx].static_val = calculated_val;
                    symbol_table[found_idx].has_static_val = 1;
                }
            }
        }
    }
}

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
                    val = LLVMBuildLoad2(builder, LLVMInt32Type(), val, "load_val");
                }
                break;
            }
        }
    }

    if(!val && loop_top >= 0 && strcmp(clean_name, "i") == 0) {
        val = LLVMBuildLoad2(builder, LLVMInt32Type(), loop_stack[loop_top].i_ptr, "load_val");
    }

    if(!val && (isdigit(clean_name[0]) || clean_name[0] == '-') && !is_literal) {
        val = LLVMConstInt(LLVMInt32Type(), atoi(clean_name), 0);
    }

    if(!val) {
        val = LLVMBuildGlobalStringPtr(builder, clean_name, "str_lit");
        is_string = 1;
    }

    if(val) {
        const char* fmt_str = is_string ? "%s\n" : "%d\n";
        LLVMValueRef fmt = LLVMBuildGlobalStringPtr(builder, fmt_str, "fmt");
        LLVMValueRef args[] = { fmt, val };
        LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
    }
}

void tamizhi_gen_ternary(char* res_name, char* v1, char* op, char* v2, char* true_val, char* false_val) {
    char clean_res[100], clean_v1[100], clean_v2[100], clean_t[100], clean_f[100];
    snprintf(clean_res, sizeof(clean_res), "%s", res_name); tamizhi_codegen_trim(clean_res);
    snprintf(clean_v1, sizeof(clean_v1), "%s", v1); tamizhi_codegen_trim(clean_v1);
    snprintf(clean_v2, sizeof(clean_v2), "%s", v2); tamizhi_codegen_trim(clean_v2);
    snprintf(clean_t, sizeof(clean_t), "%s", true_val); tamizhi_codegen_trim(clean_t);
    snprintf(clean_f, sizeof(clean_f), "%s", false_val); tamizhi_codegen_trim(clean_f);

    LLVMValueRef val1 = NULL, val2 = NULL;

    if (isdigit(clean_v1[0]) || clean_v1[0] == '-') {
        val1 = LLVMConstInt(LLVMInt32Type(), atoi(clean_v1), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_v1) == 0) {
                val1 = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "t_v1");
                break;
            }
        }
    }

    if (isdigit(clean_v2[0]) || clean_v2[0] == '-') {
        val2 = LLVMConstInt(LLVMInt32Type(), atoi(clean_v2), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_v2) == 0) {
                val2 = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "t_v2");
                break;
            }
        }
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

    LLVMValueRef cond = LLVMBuildICmp(builder, pred, val1, val2, "ternary_cond");

    LLVMValueRef t_val = NULL, f_val = NULL;

    if (isdigit(clean_t[0]) || clean_t[0] == '-') {
        t_val = LLVMConstInt(LLVMInt32Type(), atoi(clean_t), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_t) == 0) {
                t_val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "t_val");
                break;
            }
        }
    }

    char *semi_p = strchr(clean_f, ';');
    if (semi_p != NULL) *semi_p = '\0';
    tamizhi_codegen_trim(clean_f);

    if (isdigit(clean_f[0]) || clean_f[0] == '-') {
        f_val = LLVMConstInt(LLVMInt32Type(), atoi(clean_f), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_f) == 0) {
                f_val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "f_val");
                break;
            }
        }
    }

    if (!t_val) t_val = LLVMConstInt(LLVMInt32Type(), 0, 0);
    if (!f_val) f_val = LLVMConstInt(LLVMInt32Type(), 0, 0);

    LLVMValueRef select_res = LLVMBuildSelect(builder, cond, t_val, f_val, "ternary_sel");

    LLVMValueRef target_ptr = NULL;
    int target_idx = -1;
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, clean_res) == 0) {
            target_ptr = symbol_table[i].alloca_ptr;
            target_idx = i;
            break;
        }
    }
    if (!target_ptr && var_count < 100) {
        LLVMValueRef func = LLVMGetNamedFunction(module, "main");
        target_ptr = create_entry_alloca(func, LLVMInt32Type(), clean_res);
        snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
        symbol_table[var_count].alloca_ptr = target_ptr;
        symbol_table[var_count].is_str_type = 0;
        target_idx = var_count;
        var_count++;
    }
    if (target_ptr && target_idx != -1) {
        LLVMBuildStore(builder, select_res, target_ptr);
        symbol_table[target_idx].has_static_val = 0; 
    }
}

void tamizhi_gen_loop_start(int limit) {
    LLVMValueRef func = LLVMGetNamedFunction(module, "main");
    loop_top++;
    LoopContext* ctx = &loop_stack[loop_top];

    char cond_name[32], body_name[32], after_name[32];
    snprintf(cond_name, sizeof(cond_name), "loop_cond_%d", loop_counter);
    snprintf(body_name, sizeof(body_name), "loop_body_%d", loop_counter);
    snprintf(after_name, sizeof(after_name), "loop_after_%d", loop_counter);
    loop_counter++;

    ctx->cond_block = LLVMAppendBasicBlock(func, cond_name);
    ctx->body_block = LLVMAppendBasicBlock(func, body_name);
    ctx->after_block = LLVMAppendBasicBlock(func, after_name);

    ctx->i_ptr = create_entry_alloca(func, LLVMInt32Type(), "i");
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), 0, 0), ctx->i_ptr);
    LLVMBuildBr(builder, ctx->cond_block);

    LLVMPositionBuilderAtEnd(builder, ctx->cond_block);
    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), ctx->i_ptr, "i_val");
    LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntSLT, i_val, LLVMConstInt(LLVMInt32Type(), limit, 0), "loop_cond");
    LLVMBuildCondBr(builder, cond, ctx->body_block, ctx->after_block);

    LLVMPositionBuilderAtEnd(builder, ctx->body_block);
}

void tamizhi_gen_loop_end() {
    if(loop_top < 0) return;
    LoopContext* ctx = &loop_stack[loop_top];

    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), ctx->i_ptr, "i_val");
    LLVMValueRef next_val = LLVMBuildAdd(builder, i_val, LLVMConstInt(LLVMInt32Type(), 1, 0), "next_i");
    LLVMBuildStore(builder, next_val, ctx->i_ptr);

    LLVMBuildBr(builder, ctx->cond_block); 
    LLVMPositionBuilderAtEnd(builder, ctx->after_block);
    loop_top--;
}

void tamizhi_codegen_destroy() {
    if(target_machine) LLVMDisposeTargetMachine(target_machine);
    if(builder) LLVMDisposeBuilder(builder);
    if(module) LLVMDisposeModule(module);
}

void tamizhi_codegen_finish() {
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) {
        LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    }

    tamizhi_generate_universal_bitcode("output.bc");
    char *error = NULL;
    const char *out_file = "output.o";
    if (target_machine) {
        if (LLVMTargetMachineEmitToFile(target_machine, module, (char*)out_file, LLVMObjectFile, &error)) {
            fprintf(stderr, " [