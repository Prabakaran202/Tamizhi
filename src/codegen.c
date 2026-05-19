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

// 🌟 DNA-VM ஸ்டோரேஜ் குறியாக்கத்திற்கான எக்ஸ்டர்னல் பங்க்ஷன் லிங்க்
extern void encode_logic(const char* input_path, const char* output_path);

// 🌟 எல்எல்விஎம் இன்ஜினின் குளோபல் ரெஃபரன்ஸ் பாயிண்டர்கள்
LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMTargetMachineRef target_machine = NULL; 
LLVMTypeRef printf_type;
LLVMValueRef printf_func;
LLVMValueRef i_ptr = NULL; 

// 🌟 இஃப்-எல்ஸ் கண்டிஷனுக்கான பேசிக் பிளாக் பாயிண்டர்கள்
LLVMBasicBlockRef then_block, else_block, merge_block;

// 🌟 மல்டிபிள் லூப்களை டிராக்கிங் செய்ய உதவும் லூப் கவுண்ட்டர்
int loop_counter = 0;

// 🌟 கம்பைலரின் குளோபல் சிம்பல் டேபிள் (Symbol Table) கட்டமைப்பு
typedef struct {
    char name[50];
    LLVMValueRef alloca_ptr;
    int is_str_type;
    int has_static_val;
    int static_val;
} Variable;

Variable symbol_table[100];
int var_count = 0;

// 🌟 பயனர் வரையறுக்கும் பங்க்ஷன்களுக்கான சிம்பல் டேபிள் கட்டமைப்பு
typedef struct {
    char name[50];
    LLVMValueRef func_ref;
} TamizhiFunction;

TamizhiFunction function_table[50];
int func_count = 0;

// 🌟 எல்எல்விஎம் பிட்கோடு (.bc) மற்றும் ஹியூமன்-ரீடபிள் அசெம்பிளி (.ll) கோப்புகளை உருவாக்கும் பங்க்ஷன்
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

// 🌟 உருவாக்கப்பட்ட மெஷின் பைனரியை செக்யூர் DNA-VM ஸ்டோரேஜாக மாற்றும் பங்க்ஷன்
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

// 🌟 எல்எல்விஎம் ஆர்கிடெக்சர் மற்றும் டார்கெட் மெஷின் அமைப்புகளைத் தொடங்கும் பங்க்ஷன்
void tamizhi_codegen_init() {
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllAsmPrinters();

    module = LLVMModuleCreateWithName("tamizhi_engine");
    builder = LLVMCreateBuilder();

    char *target_triple = LLVMGetDefaultTargetTriple();
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
    LLVMDisposeMessage(target_triple);
}

// 🌟 கம்பைலரின் பிரதான எண்ட்ரி பாயிண்ட்டான மெயின் (main) பங்க்ஷனை பில்ட் செய்யும் இடம்
void tamizhi_generate_entry() {
    if (LLVMGetNamedFunction(module, "main")) return; 
    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_func_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
}

// 🌟 முழு எண்களுக்கான வேரியபிள்களை மெமரியில் உருவாக்கி சிம்பல் டேபிளில் சேமிக்கும் பங்க்ஷன்
void tamizhi_gen_var(char* name, int value) {
    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, name) == 0) {
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
    LLVMValueRef alloca = LLVMBuildAlloca(builder, LLVMInt32Type(), name);
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), value, 0), alloca);
    strcpy(symbol_table[var_count].name, name);
    symbol_table[var_count].alloca_ptr = alloca;
    symbol_table[var_count].is_str_type = 0;
    symbol_table[var_count].static_val = value;
    symbol_table[var_count].has_static_val = 1;
    var_count++;
}

// 🌟 சரங்களை (Strings) குளோபல் மெமரி பாயிண்டராக டிக்ளேர் செய்யும் பங்க்ஷன்
void tamizhi_gen_str(char* name, char* value) {
    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, name) == 0) {
            LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(builder, value, "str_lit");
            symbol_table[i].alloca_ptr = str_ptr;
            symbol_table[i].is_str_type = 1;
            symbol_table[i].has_static_val = 0;
            return;
        }
    }
    if (var_count >= 100) return;
    LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(builder, value, "str_lit");
    strcpy(symbol_table[var_count].name, name);
    symbol_table[var_count].alloca_ptr = str_ptr; 
    symbol_table[var_count].is_str_type = 1;
    symbol_table[var_count].has_static_val = 0;
    var_count++;
}

// 🌟 கணித கணக்கீடுகளை (Addition, Subtraction, Multiplication, Division) கம்பைல் செய்யும் பிரதான இன்ஜின்
void tamizhi_gen_math_op(char* res_name, char* var1, char* op, char* var2) {
    LLVMValueRef v1_val = NULL, v2_val = NULL;
    int s_val1 = 0, s_val2 = 0;
    int f1 = 0, f2 = 0;

    if(isdigit(var1[0])) {
        v1_val = LLVMConstInt(LLVMInt32Type(), atoi(var1), 0);
        s_val1 = atoi(var1);
        f1 = 1;
    } else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, var1) == 0) {
                if(symbol_table[i].has_static_val) {
                    s_val1 = symbol_table[i].static_val;
                    f1 = 1;
                }
                v1_val = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "v1");
                break;
            }
        }
    }

    if(isdigit(var2[0])) {
        v2_val = LLVMConstInt(LLVMInt32Type(), atoi(var2), 0);
        s_val2 = atoi(var2);
        f2 = 1;
    } else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, var2) == 0) {
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
            math_res = LLVMBuildSDiv(builder, v1_val, v2_val, "div_tmp");
            if (s_val2 != 0) calculated_val = s_val1 / s_val2;
        }

        if (math_res) {
            LLVMValueRef target_ptr = NULL;
            int found_idx = -1;

            for(int i = 0; i < var_count; i++) {
                if(strcmp(symbol_table[i].name, res_name) == 0) {
                    target_ptr = symbol_table[i].alloca_ptr;
                    found_idx = i;
                    break;
                }
            }

            if(!target_ptr && var_count < 100) {
                target_ptr = LLVMBuildAlloca(builder, LLVMInt32Type(), res_name);
                strcpy(symbol_table[var_count].name, res_name);
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

// 🌟 திரையில் அவுட்புட் காட்டும் பிரிண்ட் (Print) டிரைவர் பங்க்ஷன்
void tamizhi_gen_print(char* var_name) {
    LLVMValueRef val = NULL;
    int is_string = 0;
    char clean_name[1024];
    strcpy(clean_name, var_name);

    size_t len = strlen(clean_name);
    int is_literal = 0;
    if ((clean_name[0] == '"' || clean_name[0] == '\'') && len > 2) {
        char temp[1024];
        strncpy(temp, clean_name + 1, len - 2);
        temp[len - 2] = '\0';
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

    if(!val && i_ptr && strcmp(clean_name, "i") == 0) {
        val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "load_val");
    }

    if(!val && isdigit(clean_name[0]) && !is_literal) {
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

// 🌟 இஃப் (if) நிபந்தனை பிளாக்கைத் தொடங்கும் பிரதான பங்க்ஷன்
void tamizhi_gen_if_start(char* var1, char* op, char* var2) {
    LLVMValueRef v1 = NULL, v2 = NULL;
    for(int i = 0; i < var_count; i++) {
        if(strcmp(symbol_table[i].name, var1) == 0) {
            v1 = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "v1");
            break;
        }
    }
    if(isdigit(var2[0])) {
        v2 = LLVMConstInt(LLVMInt32Type(), atoi(var2), 0);
    } else {
        for(int i = 0; i < var_count; i++) {
            if(strcmp(symbol_table[i].name, var2) == 0) {
                v2 = LLVMBuildLoad2(builder, LLVMInt32Type(), symbol_table[i].alloca_ptr, "v2");
                break;
            }
        }
    }
    if(!v1 || !v2) return;
    
    LLVMIntPredicate pred = LLVMIntEQ;
    if (strcmp(op, "<") == 0) pred = LLVMIntSLT;
    else if (strcmp(op, ">") == 0) pred = LLVMIntSGT;
    else if (strcmp(op, "==") == 0) pred = LLVMIntEQ;
    else if (strcmp(op, "!=") == 0) pred = LLVMIntNE;
    else if (strcmp(op, "<=") == 0) pred = LLVMIntSLE;
    else if (strcmp(op, ">=") == 0) pred = LLVMIntSGE;

    LLVMValueRef cond = LLVMBuildICmp(builder, pred, v1, v2, "if_cond");
    LLVMValueRef func = LLVMGetNamedFunction(module, "main");
    
    then_block = LLVMAppendBasicBlock(func, "then");
    else_block = LLVMAppendBasicBlock(func, "else");
    merge_block = LLVMAppendBasicBlock(func, "if_cont");
    
    LLVMBuildCondBr(builder, cond, then_block, else_block);
    LLVMPositionBuilderAtEnd(builder, then_block);
}

// 🌟 எல்ஸ் (else) நிபந்தனை பிளாக்கைத் தொடங்கும் பங்க்ஷன்
void tamizhi_gen_else_start() {
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) {
        LLVMBuildBr(builder, merge_block);
    }
    LLVMPositionBuilderAtEnd(builder, else_block);
}

// 🌟 இஃப்-எல்ஸ் பிளாக்குகளை முடித்து, மெயின் லீனியர் ஸ்ட்ரீமுடன் மெர்ஜ் செய்யும் பங்க்ஷன்
void tamizhi_gen_if_end() {
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) {
        LLVMBuildBr(builder, merge_block);
    }
    
    // 🌟 எல்எல்விஎம் ஃபால்-த்ரூ பாதுகாப்பு: தற்போதைய பிளாக்கில் பிராஞ்ச் இல்லை எனில் மெர்ஜ் பிளாக்கிற்கு இணைக்கிறது
    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
    if (current_bb != merge_block && LLVMGetBasicBlockTerminator(current_bb) == NULL) {
        LLVMBuildBr(builder, merge_block);
    }
    
    LLVMPositionBuilderAtEnd(builder, merge_block);
}

// 🌟 லூப்களுக்கான (Loops) கண்டிஷன் மற்றும் எண்ட்ரி பிளாக்குகளை உருவாக்கும் பங்க்ஷன்
void tamizhi_gen_loop_start(int limit) {
    LLVMValueRef func = LLVMGetNamedFunction(module, "main");

    char cond_name[32], body_name[32], after_name[32];
    sprintf(cond_name, "loop_cond_%d", loop_counter);
    sprintf(body_name, "loop_body_%d", loop_counter);
    sprintf(after_name, "loop_after_%d", loop_counter);
    loop_counter++;

    LLVMBasicBlockRef l_cond = LLVMAppendBasicBlock(func, cond_name);
    LLVMBasicBlockRef l_body = LLVMAppendBasicBlock(func, body_name);
    LLVMBasicBlockRef l_after = LLVMAppendBasicBlock(func, after_name);

    i_ptr = LLVMBuildAlloca(builder, LLVMInt32Type(), "i");
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), 0, 0), i_ptr);
    LLVMBuildBr(builder, l_cond);

    LLVMPositionBuilderAtEnd(builder, l_cond);
    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "i_val");
    LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntSLT, i_val, LLVMConstInt(LLVMInt32Type(), limit, 0), "tmp_cond");
    LLVMBuildCondBr(builder, cond, l_body, l_after);

    LLVMPositionBuilderAtEnd(builder, l_body);
}

// 🌟 லூப் இன்கிரிமென்ட் லாஜிக்கை முடித்து மீண்டும் கண்டிஷன் பிளாக்கிற்குத் திருப்பும் பங்க்ஷன்
void tamizhi_gen_loop_end() {
    LLVMBasicBlockRef current_body = LLVMGetInsertBlock(builder);

    LLVMBasicBlockRef l_cond = LLVMGetPreviousBasicBlock(current_body);
    LLVMBasicBlockRef l_after = LLVMGetNextBasicBlock(current_body);

    LLVMValueRef i_val = LLVMBuildLoad2(builder, LLVMInt32Type(), i_ptr, "i_val");
    LLVMValueRef next_val = LLVMBuildAdd(builder, i_val, LLVMConstInt(LLVMInt32Type(), 1, 0), "next_i");
    LLVMBuildStore(builder, next_val, i_ptr);

    LLVMBuildBr(builder, l_cond); 
    LLVMPositionBuilderAtEnd(builder, l_after);
}

// 🌟 கம்பைலேஷன் ஃப்ளோவை நிறைவு செய்து பைனரி மெஷின் கோடை ரன் செய்யும் இறுதி பங்க்ஷன்
void tamizhi_codegen_finish() {
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) {
        LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    }

    tamizhi_generate_universal_bitcode("output.bc");
    char *error = NULL;
    const char *out_file = "output.o";
    if (target_machine) {
        if (LLVMTargetMachineEmitToFile(target_machine, module, (char*)out_file, LLVMObjectFile, &error)) {
            fprintf(stderr, " [Codegen Error] Failed to emit machine code: %s\n", error);
            LLVMDisposeMessage(error);
        }
    }
    tamizhi_binary_to_dna_storage(out_file);
    remove(out_file);
    fprintf(stderr, "\n[Execution] Running compiled logic...\n");
    system("lli output.bc"); 
    fprintf(stderr, "\n[Codegen] --- Tamizhi Universal Engine: SUCCESS ---\n");
    LLVMDisposeBuilder(builder);
}
