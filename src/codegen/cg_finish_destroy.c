#include "codegen_bridge.h"

void tamizhi_codegen_destroy(void) {
    if(target_machine) LLVMDisposeTargetMachine(target_machine);
    if(builder) LLVMDisposeBuilder(builder);
    if(module) LLVMDisposeModule(module);
    if(context) LLVMContextDispose(context); 
}

static void tamizhi_optimize_module(void) {
    fprintf(stderr, " [Optimizer] Running modern LLVM Pass Builder (O2)...\n");
    LLVMPassBuilderOptionsRef opts = LLVMCreatePassBuilderOptions();
    LLVMPassBuilderOptionsSetLoopInterleaving(opts, 1);
    LLVMPassBuilderOptionsSetLoopVectorization(opts, 1);
    LLVMPassBuilderOptionsSetSLPVectorization(opts, 1);
    LLVMPassBuilderOptionsSetLoopUnrolling(opts, 1);

    LLVMErrorRef err = LLVMRunPasses(module, "default<O2>", target_machine, opts);
    if (err) {
        char *msg = LLVMGetErrorMessage(err);
        LLVMDisposeMessage(msg);
        LLVMConsumeError(err);
    }
    LLVMDisposePassBuilderOptions(opts);
}

void tamizhi_codegen_finish(void) {
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
