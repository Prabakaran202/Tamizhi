#ifndef CODEGEN_H
#define CODEGEN_H

#include <llvm-c/Core.h>

// கம்பைலர் தொடக்க நிலைகள்
void tamizhi_codegen_init();
void tamizhi_generate_entry();
void tamizhi_codegen_finish();

// வேரியபிள் மற்றும் லூப் லாஜிக்
void tamizhi_gen_var(char* name, int initial_value);
void tamizhi_gen_loop_test(int limit);

// --- புதிய பங்க்ஷன்: கூட்டல் மற்றும் பிரிண்ட் செய்ய ---
// இந்த வரி இல்லை என்றால் 'parser.c' ஆல் இதைப் பயன்படுத்த முடியாது
void tamizhi_gen_add_and_print(int n1, int n2); 

#endif
