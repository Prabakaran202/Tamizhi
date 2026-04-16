#ifndef CODEGEN_H
#define CODEGEN_H
#include <llvm-c/Core.h>


void tamizhi_codegen_init();
void tamizhi_generate_entry();
void tamizhi_codegen_finish();
void tamizhi_gen_var_decl(char* name, int initial_value);
void tamizhi_gen_loop_test(int limit);

#endif
