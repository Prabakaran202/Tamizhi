#ifndef CODEGEN_H
#define CODEGEN_H

#include <llvm-c/Core.h>

// 1. கம்பைலர் தொடக்க மற்றும் முடிவு நிலைகள்
void tamizhi_codegen_init();
void tamizhi_generate_entry();
void tamizhi_codegen_finish();

// 2. வேரியபிள் மேனேஜ்மென்ட்
void tamizhi_gen_var(char* name, int initial_value);
void tamizhi_gen_var_add(char* res_name, char* var1, char* var2);
// ⭐ புதிய ஸ்ட்ரிங் ஜெனரேஷன் (Issue #6 Fix)
void tamizhi_gen_str(char* name, char* value); 

// 3. அச்சிடுதல் (Printing)
void tamizhi_gen_print(char* var_name);

// 4. லூப் லாஜிக் (Looping)
void tamizhi_gen_loop_start(int limit);
void tamizhi_gen_loop_end();

// 5. நிபந்தனை லாஜிக் (If-Else Logic)
void tamizhi_gen_if_start(char* var1, char* op, char* var2);
void tamizhi_gen_else_start();
void tamizhi_gen_if_end();

#endif
