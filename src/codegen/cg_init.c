#include "codegen_bridge.h"

// =========================================================================
// 🌟 லினக்ஸ் சிஸ்டம் கால்களுக்கான (இயக்கு / system_call) குளோபல் டிராக்கர்ஸ்
// =========================================================================
//LLVMTypeRef system_type;
//LLVMValueRef system_func;

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

    // =========================================================================
    // 🌟 Standard IO Printf பங்க்ஷன் செட்டப் 
    // =========================================================================
    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8TypeInContext(context), 0) };
    printf_type = LLVMFunctionType(LLVMInt32TypeInContext(context), printf_args, 1, 1);
    printf_func = LLVMAddFunction(module, "printf", printf_type);

    // =========================================================================
    // 🚀 [System Call Integration Fix]: லினக்ஸ் libc system() மேப்பிங்
    // =========================================================================
    // system() பங்க்ஷன் ஒரு ஸ்ட்ரிங் பாயிண்டரை இன்புட்டா வாங்கி இன்டிஜர் ரிட்டர்ன் பண்ணும்
    LLVMTypeRef system_args[] = { LLVMPointerType(LLVMInt8TypeInContext(context), 0) };
    system_type = LLVMFunctionType(LLVMInt32TypeInContext(context), system_args, 1, 0);
    system_func = LLVMAddFunction(module, "system", system_type);
    if (tamizhi_debug_mode) {
    fprintf(stderr,"[Codegen] LLVM Engine & System Call Linker Initialized for Target: %s\n", target_triple);
    }


    #ifdef __ANDROID__
    free(target_triple);
    #else
    LLVMDisposeMessage(target_triple);
    #endif
}
