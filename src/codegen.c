#include "codegen_bridge.h"

// குளோபல் மாறிகளின் மெமரி அலோகேஷன்
LLVMTypeRef system_type;
LLVMValueRef system_func;

LLVMContextRef context; 
LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMTargetMachineRef target_machine = NULL; 
LLVMTypeRef printf_type;
LLVMValueRef printf_func;
LLVMValueRef current_function = NULL; 

int loop_counter = 0;
int if_counter = 0; 

LoopContext loop_stack[MAX_LOOPS];
int loop_top = -1;

IfContext if_stack[MAX_IF_DEPTH];
int if_top = -1;

Variable symbol_table[MAX_VARS];
int var_count = 0;

TamizhiFunction function_table[MAX_FUNCS];
int func_count = 0;
