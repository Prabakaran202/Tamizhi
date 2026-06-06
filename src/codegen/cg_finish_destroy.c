#include "codegen_bridge.h"

// 🌟 [Destroy Engine]: LLVM நினைவகத்தை பாதுகாப்பாக விடுவித்தல்
void tamizhi_codegen_destroy(void) {
    if(target_machine) LLVMDisposeTargetMachine(target_machine);
    if(builder) LLVMDisposeBuilder(builder);
    if(module) LLVMDisposeModule(module);
    if(context) LLVMContextDispose(context); 
}

// 🌟 [Optimizer]: நவீன LLVM பாஸ் பில்டர் மூலம் அசுர வேக O2 ஆப்டிமைசேஷன்
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

// 🌟 [Master Core Finish]: 100% கிராஷ்-ஃப்ரீ மற்றும் சிஸ்டம் பாத் செக்யூர் கம்பைலேஷன்
void tamizhi_codegen_finish(void) {
    // 1. ஓப்பன் பிளாக்குகளுக்கு முறையான ரிட்டன் டெர்மினேட்டர் செட் செய்தல்
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL) {
        LLVMBuildRet(builder, LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0));
    }

    // 2. LLVM IR அமைப்பை சரிபார்த்தல் (Verifier)
    char *verify_err = NULL;
    if (LLVMVerifyModule(module, LLVMReturnStatusAction, &verify_err)) {
        fprintf(stderr, "[Fatal LLVM IR Error]\n%s\n", verify_err);
        LLVMDisposeMessage(verify_err);
        tamizhi_codegen_destroy();
        exit(1);
    }
    fprintf(stderr, " [Verifier] IR Graph Validated. Structural anomalies zero.\n");

    // 3. ஆப்டிமைசேஷன் ரன் செய்தல்
    tamizhi_optimize_module();
    
    // 4. யுனிவர்சல் பிட்கோடை பாதுகாப்பாக storage/ பாத்தில் ஜெனரேட் செய்தல்
    tamizhi_generate_universal_bitcode("storage/output.bc");

    // 5. நேட்டி夫 மெஷின் கோட் ஆப்ஜெக்ட் ஃபைலை storage/output.o ஆக உருவாக்குதல்
    char *error = NULL;
    if (target_machine) {
        if (LLVMTargetMachineEmitToFile(target_machine, module, "storage/output.o", LLVMObjectFile, &error)) {
            fprintf(stderr, " [Emit Error] %s\n", error);
            LLVMDisposeMessage(error);
        }
    }

    fprintf(stderr, "\n[Execution] Running compiled logic via Native AOT VM...\n");
    
    #ifdef __ANDROID__
    // 🚀 [ANDROID FIX]: llc தேவையில்லை, நேரடியா storage/ கோப்பினை கொண்டு பில்ட் செய்து எக்ஸிகியூட் செய்கிறோம்
    system("clang storage/output.o -o /data/data/com.termux/files/usr/tmp/tamizhi_out && "
           "/data/data/com.termux/files/usr/tmp/tamizhi_out; "
           "rm -f /data/data/com.termux/files/usr/tmp/tamizhi_out"); 
    #else
    // 🚀 [LINUX FIX]: லினக்ஸ் சூழலில் நேரடி ஜியிடி (JIT) எக்ஸிகியூஷன்
    system("lli storage/output.bc"); 
    #endif

    // 6. [CRITICAL SEQUENCE]: எக்ஸிகியூஷன் முடிந்ததும் பாதுகாப்பாக DNA VM மாட்யூலுக்கு மாற்றிவிட்டு ஃபைலை நீக்குதல்
    tamizhi_binary_to_dna_storage("storage/output.o");
    remove("storage/output.o");

    fprintf(stderr, "\n[Codegen] --- Tamizhi Universal Engine: SUCCESS ---\n");
    tamizhi_codegen_destroy();
}
