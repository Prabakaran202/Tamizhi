#include "codegen.h"
#include <llvm-c/Core.h>
#include <stdio.h>

extern LLVMModuleRef module;
extern LLVMContextRef context;
extern LLVMBuilderRef builder;

// 1. Listen Function (மாற்றமில்லை)
void tamizhi_gen_socket_listen(int port) {
    LLVMTypeRef param_types[] = { LLVMInt32TypeInContext(context) };
    LLVMTypeRef func_type = LLVMFunctionType(LLVMVoidTypeInContext(context), param_types, 1, 0);

    LLVMValueRef func = LLVMGetNamedFunction(module, "tamizhi_rt_listen");
    if (!func) {
        func = LLVMAddFunction(module, "tamizhi_rt_listen", func_type);
    }

    LLVMValueRef port_val = LLVMConstInt(LLVMInt32TypeInContext(context), port, 0);
    LLVMValueRef args[] = { port_val };
    LLVMBuildCall2(builder, func_type, func, args, 1, "");
}

// 2. Accept Function (🔥 UPDATE: Void-க்கு பதிலாக Int32 ரிட்டர்ன் செய்கிறது)
LLVMValueRef tamizhi_gen_socket_accept() {
    // LLVM-ல் external ஃபங்ஷனை Declare செய்தல்: int tamizhi_rt_accept()
    LLVMTypeRef func_type = LLVMFunctionType(LLVMInt32TypeInContext(context), NULL, 0, 0);

    LLVMValueRef func = LLVMGetNamedFunction(module, "tamizhi_rt_accept");
    if (!func) {
        func = LLVMAddFunction(module, "tamizhi_rt_accept", func_type);
    }

    // ஃபங்ஷனை கால் செய்து, கிடைக்கும் client socket-ஐ ரிட்டர்ன் செய்கிறோம்
    return LLVMBuildCall2(builder, func_type, func, NULL, 0, "client_socket");
}

// 3. Send Response Function (🚀 NEW: டைனமிக் HTTP ரெஸ்பான்ஸை LLVM-ல் இணைத்தல்)
void tamizhi_gen_socket_send_response(LLVMValueRef client_socket, int status_code, const char* message) {
    // ஃபங்ஷன் சிக்னேச்சர்: void tamizhi_rt_send_response(int, int, char*)
    LLVMTypeRef param_types[] = { 
        LLVMInt32TypeInContext(context),                   // client_socket (int)
        LLVMInt32TypeInContext(context),                   // status_code (int)
        LLVMPointerType(LLVMInt8TypeInContext(context), 0) // content (char*)
    };
    LLVMTypeRef func_type = LLVMFunctionType(LLVMVoidTypeInContext(context), param_types, 3, 0);

    LLVMValueRef func = LLVMGetNamedFunction(module, "tamizhi_rt_send_response");
    if (!func) {
        func = LLVMAddFunction(module, "tamizhi_rt_send_response", func_type);
    }

    // ஆர்க்யுமென்ட்களை LLVM Value ஆக மாற்றுதல்
    LLVMValueRef status_val = LLVMConstInt(LLVMInt32TypeInContext(context), status_code, 0);
    LLVMValueRef content_val = LLVMBuildGlobalStringPtr(builder, message, "resp_str");

    LLVMValueRef args[] = { client_socket, status_val, content_val };
    LLVMBuildCall2(builder, func_type, func, args, 3, "");
}
