#include "codegen.h"
#include "ast.h" // 🌟 AST மரத்தின் அமைப்பை உள்ளே கொண்டு வருகிறோம்
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Target.h>
#include <llvm-c/BitWriter.h> 
#include <llvm-c/Transforms/PassBuilder.h> // 🌟 Modern LLVM 19+ Pass Builder
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* 🌟 நவீன LLVM 19/21 ஆர்கிடெக்சரில் லெகசி Transforms/Scalar.h மற்றும் Utils.h ஹெடர்கள் 
   முழுமையாக நீக்கப்பட்டுவிட்டதால், பில்ட் எரரைத் தவிர்க்க அவை இங்கிருந்து கம்ப்ளீட்டாக அகற்றப்பட்டுள்ளன. */

extern void encode_logic(const char* input_path, const char* output_path);

// 🌟 லெக்சரிலிருந்து உலகளாவிய தற்போதைய வரி எண் மாறியை வாங்குகிறோம்
extern int current_line; 

// ==========================================
// 🌟 மாடர்ன் LLVM குளோபல் கான்டெக்สต์ செட்டப் 
// ==========================================
LLVMContextRef context; // 🌟 உலகளாவிய புதிய கான்டெக்ஸ்ட் ரெஃபரன்ஸ்
LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMTargetMachineRef target_machine = NULL; 
LLVMTypeRef printf_type;
LLVMValueRef printf_func;

// 🌟 [MASTER LINKER FIX]: லிங்கர் எரரைத் தீர்க்கும் உலகளாவிய தற்போதைய ஃபங்ஷன் பாயிண்டர்!
LLVMValueRef current_function = NULL; 

int loop_counter = 0;
int if_counter = 0; // 🌟 இஃப்-எல்ஸ் பிளாக்குகளுக்கான உலகளாவிய கவுண்டர்

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

// ==========================================
// 🌟 [v0.1.5 FIX]: அண்டர்ஃபுளோ பாதுகாப்பு கொண்ட இஃப்-எல்ஸ் ஸ்டேக் கட்டமைப்பு!
// ==========================================
#define MAX_IF_DEPTH 256
typedef struct {
    LLVMBasicBlockRef true_block;
    LLVMBasicBlockRef false_block;
    LLVMBasicBlockRef end_block;
    int has_else;
} IfContext;

IfContext if_stack[MAX_IF_DEPTH];
int if_top = -1;

// ==========================================
// 🌟 சிம்பல் மற்றும் ஃபங்ஷன் டேபிள் பவுண்ட்ஸ் செட்டிங்ஸ்
// ==========================================
#define MAX_VARS 100
#define MAX_FUNCS 50

typedef struct {
    char name[100]; 
    LLVMValueRef alloca_ptr;
    int is_str_type;
    int has_static_val;
    int static_val;
} Variable;

Variable symbol_table[MAX_VARS];
int var_count = 0;

typedef struct {
    char name[100];
    LLVMValueRef func_ref;
    LLVMBasicBlockRef resume_block; // 🌟 ஒவ்வொரு ஃபங்ஷனோட ஓபன் பிளாக்கைக் கண்காணிக்க
} TamizhiFunction; 

TamizhiFunction function_table[MAX_FUNCS];
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
// 🌟 Android ARM64 பாயிண்டர் சிதைவு இல்லாத entry-block அலோகேஷன்
// ======================================================================
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
    }

    return alloca;
}

// =========================================================================
// 🌳 AST TREE WALKER (AST மரத்தை LLVM-IR ஆக மாற்றும் இன்ஜின்) 🌳
// =========================================================================

LLVMValueRef tamizhi_evaluate_ast(ASTNode* node) {
    if (node == NULL) return NULL;

    if (node->type == AST_NUMBER) {
        return LLVMConstInt(LLVMInt32TypeInContext(context), node->data.num_value, 0);
    }
    else if (node->type == AST_IDENTIFIER) {
        char clean_name[256];
        snprintf(clean_name, sizeof(clean_name), "%s", node->data.var_name);
        tamizhi_codegen_trim(clean_name);

        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_name) == 0) {
                return LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "ast_load");
            }
        }
        fprintf(stderr, "[Codegen Error] மாறி கிடைக்கவில்லை: '%s'\n", clean_name);
        return LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0); 
    }
    else if (node->type == AST_BINARY_OP) {
        LLVMValueRef left_val = tamizhi_evaluate_ast(node->data.binop.left);
        LLVMValueRef right_val = tamizhi_evaluate_ast(node->data.binop.right);

        if (!left_val || !right_val) return NULL;

        if (strcmp(node->data.binop.op, "+") == 0) {
            return LLVMBuildAdd(builder, left_val, right_val, "ast_add");
        } else if (strcmp(node->data.binop.op, "-") == 0) {
            return LLVMBuildSub(builder, left_val, right_val, "ast_sub");
        } else if (strcmp(node->data.binop.op, "*") == 0) {
            return LLVMBuildMul(builder, left_val, right_val, "ast_mul");
        } else if (strcmp(node->data.binop.op, "/") == 0) {
            return LLVMBuildSDiv(builder, left_val, right_val, "ast_div");
        }
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

    if (!target_ptr && var_count < MAX_VARS) {
        if (current_function == NULL) {
            current_function = LLVMGetNamedFunction(module, "main");
        }
        target_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), clean_res);
        snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
        symbol_table[var_count].alloca_ptr = target_ptr;
        symbol_table[var_count].is_str_type = 0;
        found_idx = var_count;
        var_count++;
    }

    if (target_ptr && found_idx != -1) {
        LLVMBuildStore(builder, math_res, target_ptr);
        symbol_table[found_idx].has_static_val = 0; 
    }
}
// =========================================================================

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
    if (!filename) {
        fprintf(stderr, " [DNA-VM Error] filename is NULL!\n");
        return;
    }
    FILE *check = fopen(filename, "rb");
    if (check) {
        fclose(check);
        encode_logic(filename, "storage/project_binary.dna"); 
        fprintf(stderr, " [DNA-VM] Binary AOT Secured at: storage/project_binary.dna\n");
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

    context = LLVMContextCreate();
    module = LLVMModuleCreateWithNameInContext("tamizhi_engine", context);
    builder = LLVMCreateBuilderInContext(context);

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

    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8TypeInContext(context), 0) };
    printf_type = LLVMFunctionType(LLVMInt32TypeInContext(context), printf_args, 1, 1);
    printf_func = LLVMAddFunction(module, "printf", printf_type);

    fprintf(stderr, " [Codegen] LLVM Engine Initialized with Target: %s\n", target_triple);

    #ifdef __ANDROID__
    free(target_triple);
    #else
    LLVMDisposeMessage(target_triple);
    #endif
}

void tamizhi_gen_function_start(char* func_name) {
    char clean_name[100];
    snprintf(clean_name, sizeof(clean_name), "%s", func_name);
    tamizhi_codegen_trim(clean_name);

    if (LLVMGetNamedFunction(module, clean_name)) return;

    LLVMTypeRef func_type = LLVMFunctionType(LLVMInt32TypeInContext(context), NULL, 0, 0);
    LLVMValueRef function = LLVMAddFunction(module, clean_name, func_type);

    current_function = function; 

    LLVMBasicBlockRef entry_block = LLVMAppendBasicBlockInContext(context, function, "entry");
    LLVMPositionBuilderAtEnd(builder, entry_block);

    if (func_count < MAX_FUNCS) {
        snprintf(function_table[func_count].name, sizeof(function_table[func_count].name), "%s", clean_name);
        function_table[func_count].func_ref = function;
        function_table[func_count].resume_block = entry_block;
        func_count++;
    }
}

void tamizhi_gen_function_end() {
    // 🌟 [CRITICAL FIX]: i32 ரிட்டன் டைப்பிற்கு இணங்க கச்சிதமாக i32 0 ரிட்டன் செய்கிறோம்!
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) {
        LLVMBuildRet(builder, LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0));
    }

    LLVMValueRef main_fn = LLVMGetNamedFunction(module, "main");
    current_function = main_fn;
    if (main_fn) {
        LLVMBasicBlockRef bb = LLVMGetFirstBasicBlock(main_fn);
        LLVMBasicBlockRef open_bb = NULL;
        while (bb) {
            if (LLVMGetBasicBlockTerminator(bb) == NULL) {
                open_bb = bb;
                break;
            }
            bb = LLVMGetNextBasicBlock(bb);
        }
        if (open_bb) {
            LLVMPositionBuilderAtEnd(builder, open_bb);
        } else {
            LLVMBasicBlockRef cont = LLVMAppendBasicBlockInContext(context, main_fn, "main_cont");
            LLVMPositionBuilderAtEnd(builder, cont);
        }
    }
}

void tamizhi_gen_function_call(char* func_name) {
    char clean_name[100];
    snprintf(clean_name, sizeof(clean_name), "%s", func_name);
    tamizhi_codegen_trim(clean_name);

    LLVMValueRef target_func = LLVMGetNamedFunction(module, clean_name);
    if (target_func) {
        LLVMTypeRef func_type = LLVMFunctionType(LLVMInt32TypeInContext(context), NULL, 0, 0);
        LLVMBuildCall2(builder, func_type, target_func, NULL, 0, "call_tmp");
    } else {
        fprintf(stderr, "[Codegen Error] ಫಂಗ್ಷன் வரையறுக்கப்படவில்லை: '%s'\n", clean_name);
    }
}

void tamizhi_generate_entry() {
    if (LLVMGetNamedFunction(module, "main")) return; 

    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32TypeInContext(context), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);

    current_function = main_func; 

    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(context, main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry); 
}

void tamizhi_gen_var(char* name, int value) {
    char clean_res[100];
    snprintf(clean_res, sizeof(clean_res), "%s", name);
    tamizhi_codegen_trim(clean_res);

    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, clean_res) == 0) {
            symbol_table[i].static_val = value;
            symbol_table[i].has_static_val = 1;
            symbol_table[i].is_str_type = 0;
            if(symbol_table[i].alloca_ptr) {
                LLVMBuildStore(builder, LLVMConstInt(LLVMInt32TypeInContext(context), value, 0), symbol_table[i].alloca_ptr);
            }
            return;
        }
    }
    if (var_count >= MAX_VARS) {
        fprintf(stderr, "[Codegen Error] Symbol table full! Cannot add '%s'\n", clean_res);
        return;
    }

    if (current_function == NULL) {
        current_function = LLVMGetNamedFunction(module, "main");
    }
    LLVMValueRef alloca = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), clean_res);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32TypeInContext(context), value, 0), alloca);

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
    if (var_count >= MAX_VARS) {
        fprintf(stderr, "[Codegen Error] Symbol table full! Cannot add '%s'\n", clean_res);
        return;
    }
    LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(builder, value, "str_lit");
    snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_res);
    symbol_table[var_count].alloca_ptr = str_ptr; 
    symbol_table[var_count].is_str_type = 1;
    symbol_table[var_count].has_static_val = 0;
    var_count++;
}

void tamizhi_gen_math_op(char* res_name, char* v1, char* op, char* var2) {
    char clean_res[100], clean_v1[100], clean_v2[100];
    snprintf(clean_res, sizeof(clean_res), "%s", res_name); tamizhi_codegen_trim(clean_res);
    snprintf(clean_v1, sizeof(clean_v1), "%s", v1); tamizhi_codegen_trim(clean_v1);
    snprintf(clean_v2, sizeof(clean_v2), "%s", var2); tamizhi_codegen_trim(clean_v2);

    LLVMValueRef v1_val = NULL, v2_val = NULL;
    int s_val1 = 0, s_val2 = 0;
    int f1 = 0, f2 = 0;

    if(isdigit((unsigned char)clean_v1[0]) || clean_v1[0] == '-') {
        v1_val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_v1), 0);
        s_val1 = atoi(clean_v1);
        f1 = 1;
    } else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, clean_v1) == 0) {
                if(symbol_table[i].has_static_val) {
                    s_val1 = symbol_table[i].static_val;
                    f1 = 1;
                }
                v1_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "v1");
                break;
            }
        }
    }

    if(isdigit((unsigned char)clean_v2[0]) || clean_v2[0] == '-') {
        v2_val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_v2), 0);
        s_val2 = atoi(clean_v2);
        f2 = 1;
    } else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, clean_v2) == 0) {
                if(symbol_table[i].has_static_val) {
                    s_val2 = symbol_table[i].static_val;
                    f2 = 1;
                }
                v2_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "v2");
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
            if(f2 && s_val2 == 0) {
                fprintf(stderr, "[Runtime Error] வரி %d: பூஜ்ஜியத்தால் வகுக்க முடியாது!\n", current_line);
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

            if(!target_ptr && var_count < MAX_VARS) {
                if (current_function == NULL) {
                    current_function = LLVMGetNamedFunction(module, "main");
                }
                target_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), clean_res);
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

    int len = strlen(clean_name);
    if (clean_name[0] == '"' && clean_name[len - 1] == '"' && len >= 2) {
        char temp[1024];
        memset(temp, 0, sizeof(temp));
        strncpy(temp, clean_name + 1, len - 2);
        strcpy(clean_name, temp);
        is_literal = 1;
        is_string = 1;
    }

    if (!is_literal) {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, clean_name) == 0) {
                val = symbol_table[i].alloca_ptr;
                if (symbol_table[i].is_str_type) {
                    is_string = 1;
                } else {
                    val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), val, "load_val");
                }
                break;
            }
        }
    }

    if (!val && strcmp(clean_name, "i") == 0) {
        for (int lvl = loop_top; lvl >= 0; lvl--) {
            if (loop_stack[lvl].i_ptr) {
                val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), loop_stack[lvl].i_ptr, "load_loop_i");
                break;
            }
        }
    }

    if(!val && (isdigit((unsigned char)clean_name[0]) || clean_name[0] == '-') && !is_literal) {
        val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_name), 0);
    }

    if(!val) {
        val = LLVMBuildGlobalStringPtr(builder, clean_name, "str_lit");
        is_string = 1;
    }

    if(val) {
        const char* fmt = is_string ? "%s\n" : "%d\n";
        LLVMValueRef fmt_ref = LLVMBuildGlobalStringPtr(builder, fmt, "fmt");
        LLVMValueRef args[] = { fmt_ref, val };
        LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "print_call");
    }
}

void tamizhi_gen_ternary(char* res_name, char* v1, char* op, char* var2, char* true_val, char* false_val) {
    char clean_res[100], clean_v1[100], clean_v2[100], clean_t[100], clean_f[100];
    snprintf(clean_res, sizeof(clean_res), "%s", res_name); tamizhi_codegen_trim(clean_res);
    snprintf(clean_v1, sizeof(clean_v1), "%s", v1); tamizhi_codegen_trim(clean_v1);
    snprintf(clean_v2, sizeof(clean_v2), "%s", var2); tamizhi_codegen_trim(clean_v2);
    snprintf(clean_t, sizeof(clean_t), "%s", true_val); tamizhi_codegen_trim(clean_t);
    snprintf(clean_f, sizeof(clean_f), "%s", false_val); tamizhi_codegen_trim(clean_f);

    LLVMValueRef val1 = NULL, val2 = NULL;

    if (isdigit((unsigned char)clean_v1[0]) || clean_v1[0] == '-') {
        val1 = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_v1), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_v1) == 0) {
                val1 = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "t_v1");
                break;
            }
        }
    }

    if (isdigit((unsigned char)clean_v2[0]) || clean_v2[0] == '-') {
        val2 = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_v2), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_v2) == 0) {
                val2 = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "t_v2");
                break;
            }
        }
    }

    if (!val1) val1 = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    if (!val2) val2 = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);

    LLVMIntPredicate pred = LLVMIntEQ;
    if (strcmp(op, "<") == 0) pred = LLVMIntSLT;
    else if (strcmp(op, ">") == 0) pred = LLVMIntSGT;
    else if (strcmp(op, "==") == 0) pred = LLVMIntEQ;
    else if (strcmp(op, "!=") == 0) pred = LLVMIntNE;
    else if (strcmp(op, "<=") == 0) pred = LLVMIntSLE;
    else if (strcmp(op, ">=") == 0) pred = LLVMIntSGE;

    LLVMValueRef cond = LLVMBuildICmp(builder, pred, val1, val2, "ternary_cond");

    LLVMValueRef t_val = NULL, f_val = NULL;

    if (isdigit((unsigned char)clean_t[0]) || clean_t[0] == '-') {
        t_val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_t), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_t) == 0) {
                t_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "t_val");
                break;
            }
        }
    }

    char *semi_p = strchr(clean_f, ';');
    if (semi_p != NULL) *semi_p = '\0';
    tamizhi_codegen_trim(clean_f);

    if (isdigit((unsigned char)clean_f[0]) || clean_f[0] == '-') {
        f_val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_f), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_f) == 0) {
                f_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "f_val");
                break;
            }
        }
    }

    if (!t_val) t_val = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    if (!f_val) f_val = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);

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
    if (!target_ptr && var_count < MAX_VARS) {
        if (current_function == NULL) {
            current_function = LLVMGetNamedFunction(module, "main");
        }
        target_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), clean_res);
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
    if (current_function == NULL) {
        current_function = LLVMGetNamedFunction(module, "main");
    }
    loop_top++;
    LoopContext* ctx = &loop_stack[loop_top];

    char cond_name[32], body_name[32], after_name[32], i_name[32];
    snprintf(cond_name, sizeof(cond_name), "loop_cond_%d", loop_counter);
    snprintf(body_name, sizeof(body_name), "loop_body_%d", loop_counter);
    snprintf(after_name, sizeof(after_name), "loop_after_%d", loop_counter);

    snprintf(i_name, sizeof(i_name), "__loop_i_%d", loop_counter);
    loop_counter++;

    ctx->cond_block = LLVMAppendBasicBlockInContext(context, current_function, cond_name);
    ctx->body_block = LLVMAppendBasicBlockInContext(context, current_function, body_name);
    ctx->after_block = LLVMAppendBasicBlockInContext(context, current_function, after_name);

    ctx->i_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), i_name);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0), ctx->i_ptr);
    LLVMBuildBr(builder, ctx->cond_block);

    LLVMPositionBuilderAtEnd(builder, ctx->cond_block);
    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), ctx->i_ptr, "i_val");
    LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntSLT, i_val, LLVMConstInt(LLVMInt32TypeInContext(context), limit, 0), "loop_cond");
    LLVMBuildCondBr(builder, cond, ctx->body_block, ctx->after_block);

    LLVMPositionBuilderAtEnd(builder, ctx->body_block);
}

void tamizhi_gen_loop_end() {
    if(loop_top < 0) return;
    LoopContext* ctx = &loop_stack[loop_top];

    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), ctx->i_ptr, "i_val");
    LLVMValueRef next_val = LLVMBuildAdd(builder, i_val, LLVMConstInt(LLVMInt32TypeInContext(context), 1, 0), "next_i");
    LLVMBuildStore(builder, next_val, ctx->i_ptr);

    LLVMBuildBr(builder, ctx->cond_block); 
    LLVMPositionBuilderAtEnd(builder, ctx->after_block);
    loop_top--;
}

// =========================================================================
// 🌟 [v0.1.5 MASTER CORES]: 100% பக்-ஃப்ரீ இஃப்-எல்ஸ் பிரான்சிங் இன்ஜின்!
// =========================================================================
void tamizhi_gen_if_start(char* lhs, char* rel_op, char* rhs) {
    char clean_lhs[100], clean_rhs[100], clean_op[10];
    snprintf(clean_lhs, sizeof(clean_lhs), "%s", lhs); tamizhi_codegen_trim(clean_lhs);
    snprintf(clean_rhs, sizeof(clean_rhs), "%s", rhs); tamizhi_codegen_trim(clean_rhs);
    snprintf(clean_op, sizeof(clean_op), "%s", rel_op); tamizhi_codegen_trim(clean_op);

    if_top++;
    if (if_top >= MAX_IF_DEPTH - 1) {
        fprintf(stderr, "[If Error] Maximum nested if depth exceeded\n");
        return;
    }

    if (current_function == NULL) {
        current_function = LLVMGetNamedFunction(module, "main");
    }

    LLVMValueRef val1 = NULL, val2 = NULL;

    if (isdigit((unsigned char)clean_lhs[0]) || clean_lhs[0] == '-') {
        val1 = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_lhs), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_lhs) == 0) {
                val1 = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "if_v1");
                break;
            }
        }
    }

    if (isdigit((unsigned char)clean_rhs[0]) || clean_rhs[0] == '-') {
        val2 = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_rhs), 0);
    } else {
        for (int i = 0; i < var_count; i++) {
            if (strcmp(symbol_table[i].name, clean_rhs) == 0) {
                val2 = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "if_v2");
                break;
            }
        }
    }

    if (!val1) val1 = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    if (!val2) val2 = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);

    LLVMIntPredicate pred = LLVMIntEQ;
    if (strcmp(clean_op, "<") == 0) pred = LLVMIntSLT;
    else if (strcmp(clean_op, ">") == 0) pred = LLVMIntSGT;
    else if (strcmp(clean_op, "==") == 0) pred = LLVMIntEQ;
    else if (strcmp(clean_op, "!=") == 0) pred = LLVMIntNE;
    else if (strcmp(clean_op, "<=") == 0) pred = LLVMIntSLE;
    else if (strcmp(clean_op, ">=") == 0) pred = LLVMIntSGE;

    LLVMValueRef cond = LLVMBuildICmp(builder, pred, val1, val2, "if_cond");

    char true_name[32], false_name[32], end_name[32];
    snprintf(true_name, sizeof(true_name), "if_true_%d", if_counter);
    snprintf(false_name, sizeof(false_name), "if_false_%d", if_counter);
    snprintf(end_name, sizeof(end_name), "if_end_%d", if_counter);
    if_counter++;

    if_stack[if_top].true_block = LLVMAppendBasicBlockInContext(context, current_function, true_name);
    if_stack[if_top].false_block = LLVMAppendBasicBlockInContext(context, current_function, false_name);
    if_stack[if_top].end_block = LLVMAppendBasicBlockInContext(context, current_function, end_name);
    if_stack[if_top].has_else = 0;

    LLVMBuildCondBr(builder, cond, if_stack[if_top].true_block, if_stack[if_top].false_block);
    LLVMPositionBuilderAtEnd(builder, if_stack[if_top].true_block);
}

// 🌟 [NEW CORENODE]: இஃப் பாடி முடிஞ்சதும் ஸ்ட்ரெயிட்டா எண்ட் பிளாக்குக்கு லிங்க் தர்றோம் பிரபா!
void tamizhi_gen_if_body_end() {
    if (if_top < 0) return;
    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
    if (current_bb && LLVMGetBasicBlockTerminator(current_bb) == NULL) {
        LLVMBuildBr(builder, if_stack[if_top].end_block);
    }
}

void tamizhi_gen_else_start() {
    if (if_top < 0) return;
    if_stack[if_top].has_else = 1;
    // பில்டரை முறைப்படி எல்ஸ் பிளாக்கிற்குள் கொண்டு வருகிறோம் பிரபா
    LLVMPositionBuilderAtEnd(builder, if_stack[if_top].false_block);
}

void tamizhi_gen_if_end() {
    if (if_top < 0) return;

    IfContext* ctx = &if_stack[if_top];
    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);

    // கரண்ட் பிளாக்ல டெர்மினேட்டர் இல்லனா எண்ட் பிளாக்கோட லிங்க் பண்றோம்
    if (current_bb && LLVMGetBasicBlockTerminator(current_bb) == NULL) {
        LLVMBuildBr(builder, ctx->end_block);
    }

    // எல்ஸ் பிளாக் இல்லாத பட்சத்தில், ஃபால்ஸ் பிளாக்கும் எண்ட் பிளாக்கோட கமிட் ஆகணும் பிரபா
    if (!ctx->has_else) {
        LLVMPositionBuilderAtEnd(builder, ctx->false_block);
        if (LLVMGetBasicBlockTerminator(ctx->false_block) == NULL) {
            LLVMBuildBr(builder, ctx->end_block);
        }
    }

    // அடுத்த ஸ்டேட்மென்ட்கள் ரன் ஆக பில்டரை எண்ட் பிளாக்கில் நிறுத்துகிறோம்
    LLVMPositionBuilderAtEnd(builder, ctx->end_block);
    if_top--;
}

// =========================================================================

void tamizhi_gen_return(char* return_val) {
    char clean_name[1024];
    snprintf(clean_name, sizeof(clean_name), "%s", return_val);
    tamizhi_codegen_trim(clean_name);

    LLVMValueRef ret_llvm_val = NULL;

    if (isdigit((unsigned char)clean_name[0]) || clean_name[0] == '-') {
        ret_llvm_val = LLVMConstInt(LLVMInt32TypeInContext(context), atoi(clean_name), 0);
    } 
    else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, clean_name) == 0) {
                ret_llvm_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "ret_load");
                break;
            }
        }
    }

    if (!ret_llvm_val) {
        ret_llvm_val = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    }

    LLVMValueRef target_ptr = NULL;
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, "__tamizhi_ret") == 0) {
            target_ptr = symbol_table[i].alloca_ptr;
            break;
        }
    }

    if (!target_ptr && var_count < MAX_VARS) {
        if (current_function == NULL) {
            current_function = LLVMGetNamedFunction(module, "main");
        }
        target_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), "__tamizhi_ret");
        snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "__tamizhi_ret");
        symbol_table[var_count].alloca_ptr = target_ptr;
        symbol_table[var_count].is_str_type = 0;
        var_count++;
    }

    if (target_ptr) {
        LLVMBuildStore(builder, ret_llvm_val, target_ptr);
    }
}

void tamizhi_gen_assign_from_return(char* var_name) {
    char clean_name[100];
    snprintf(clean_name, sizeof(clean_name), "%s", var_name);
    tamizhi_codegen_trim(clean_name);

    LLVMValueRef ret_llvm_val = NULL;

    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, "__tamizhi_ret") == 0) {
            ret_llvm_val = LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), symbol_table[i].alloca_ptr, "ret_val_load");
            break;
        }
    }

    if (!ret_llvm_val) {
        ret_llvm_val = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    }

    LLVMValueRef target_ptr = NULL;
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, clean_name) == 0) {
            target_ptr = symbol_table[i].alloca_ptr;
            break;
        }
    }

    if (!target_ptr && var_count < MAX_VARS) {
        if (current_function == NULL) {
            current_function = LLVMGetNamedFunction(module, "main");
        }
        target_ptr = create_entry_alloca(current_function, LLVMInt32TypeInContext(context), clean_name);
        snprintf(symbol_table[var_count].name, sizeof(symbol_table[var_count].name), "%s", clean_name);
        symbol_table[var_count].alloca_ptr = target_ptr;
        symbol_table[var_count].is_str_type = 0;
        var_count++;
    }

    if (target_ptr) {
        LLVMBuildStore(builder, ret_llvm_val, target_ptr);
    }
}

void tamizhi_codegen_destroy() {
    if(target_machine) LLVMDisposeTargetMachine(target_machine);
    if(builder) LLVMDisposeBuilder(builder);
    if(module) LLVMDisposeModule(module);
    if(context) LLVMContextDispose(context); 
}

static void tamizhi_optimize_module() {
    fprintf(stderr, " [Optimizer] Running modern LLVM Pass Builder (O2)...\n");
    LLVMPassBuilderOptionsRef opts = LLVMCreatePassBuilderOptions();
    LLVMPassBuilderOptionsSetLoopInterleaving(opts, 1);
    LLVMPassBuilderOptionsSetLoopVectorization(opts, 1);
    LLVMPassBuilderOptionsSetSLPVectorization(opts, 1);
    LLVMPassBuilderOptionsSetLoopUnrolling(opts, 1);

    LLVMErrorRef err = LLVMRunPasses(module, "default<O2>", target_machine, opts);
    if (err) {
        char *msg = LLVMGetErrorMessage(err);
        fprintf(stderr, " [Optimizer Warning] %s\n", msg);
        LLVMDisposeMessage(msg);
        LLVMConsumeError(err);
    } else {
        fprintf(stderr, " [Optimizer] O2 optimization pipeline complete.\n");
    }
    LLVMDisposePassBuilderOptions(opts);
}

void tamizhi_codegen_finish() {
    // 🌟 [GHOST 0 FIXED]: மெயின் பிளாக்கின் இறுதி எக்சிட் பாயிண்டிலும் கச்சிதமாக i32 0 ரிட்டன் தந்து வெரிஃபையர் எரரை முழுமையாகத் தீர்க்கிறோம் பிரபா!
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) {
        LLVMBuildRet(builder, LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0));
    }

    char *verify_err = NULL;
    if (LLVMVerifyModule(module, LLVMReturnStatusAction, &verify_err)) {
        fprintf(stderr, "[Fatal LLVM IR Error]\n%s\n", verify_err);
        LLVMDisposeMessage(verify_err);
        tamizhi_codegen_destroy();
        exit(1);
    }
    fprintf(stderr, " [Verifier] IR Graph Validated. Structural anomalies zero.\n");

    tamizhi_optimize_module();

    tamizhi_generate_universal_bitcode("output.bc");
    char *error = NULL;
    if (target_machine) {
        if (LLVMTargetMachineEmitToFile(target_machine, module, "output.o", LLVMObjectFile, &error)) {
            fprintf(stderr, " [Codegen Error] Failed to emit machine code: %s\n", error);
            LLVMDisposeMessage(error);
        }
    }

    tamizhi_binary_to_dna_storage("output.o");
    remove("output.o");

    fprintf(stderr, "\n[Execution] Running compiled logic via Native AOT VM...\n");
    #ifdef __ANDROID__
    system("llc output.bc -filetype=obj -o output.o && "
           "clang output.o -o /data/data/com.termux/files/usr/tmp/tamizhi_out && "
           "/data/data/com.termux/files/usr/tmp/tamizhi_out; "
           "rm -f /data/data/com.termux/files/usr/tmp/tamizhi_out"); 
    #else
    system("lli output.bc"); 
    #endif

    fprintf(stderr, "\n[Codegen] --- Tamizhi Universal Engine: SUCCESS ---\n");
    tamizhi_codegen_destroy();
}
