#include "codegen_bridge.h"

void tamizhi_codegen_init(void) {
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
