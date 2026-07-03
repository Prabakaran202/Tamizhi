#include "codegen.h"
#include <llvm-c/Core.h>
#include <stdio.h>

extern LLVMModuleRef module;
extern LLVMContextRef context;
extern LLVMBuilderRef builder;

void tamizhi_gen_socket_listen(int port) {
    // 1. LLVM-ல் external ஃபங்ஷனை Declare செய்தல்: void tamizhi_rt_listen(int)
    LLVMTypeRef param_types[] = { LLVMInt32TypeInContext(context) };
    LLVMTypeRef func_type = LLVMFunctionType(LLVMVoidTypeInContext(context), param_types, 1, 0);
    
    LLVMValueRef func = LLVMGetNamedFunction(module, "tamizhi_rt_listen");
    if (!func) {
        func = LLVMAddFunction(module, "tamizhi_rt_listen", func_type);
    }
    
    // 2. அந்த ஃபங்ஷனை ரன்-டைமில் கூப்பிடுவதற்கான (Call) இன்ஸ்ட்ரக்ஷனை உருவாக்குதல்
    LLVMValueRef port_val = LLVMConstInt(LLVMInt32TypeInContext(context), port, 0);
    LLVMValueRef args[] = { port_val };
    LLVMBuildCall2(builder, func_type, func, args, 1, "");
}

void tamizhi_gen_socket_accept() {
    // 1. LLVM-ல் external ஃபங்ஷனை Declare செய்தல்: void tamizhi_rt_accept()
    LLVMTypeRef func_type = LLVMFunctionType(LLVMVoidTypeInContext(context), NULL, 0, 0);
    
    LLVMValueRef func = LLVMGetNamedFunction(module, "tamizhi_rt_accept");
    if (!func) {
        func = LLVMAddFunction(module, "tamizhi_rt_accept", func_type);
    }
    
    // 2. அந்த ஃபங்ஷனை ரன்-டைமில் கூப்பிடுவதற்கான இன்ஸ்ட்ரக்ஷனை உருவாக்குதல்
    LLVMBuildCall2(builder, func_type, func, NULL, 0, "");
}
