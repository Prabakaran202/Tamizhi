
#include "codegen_bridge.h"  // இது மட்டும் போதும்
// =========================================================================
// 🌟 [v0.1.5 MASTER CORES]: 100% பக்-ஃப்ரீ இஃப்-எல்ஸ் பிரான்சிங் இன்ஜின்!
// =========================================================================
void tamizhi_gen_if_end() {
    // 1. அண்டர்ஃபுளோ பாதுகாப்பு செக்
    if (if_top < 0) return;

    // தற்போதைய இஃப் கான்டெக்ஸ்ட்டை லோக்கல் பஃபரில் பத்திரப்படுத்துகிறோம்
    IfContext ctx = if_stack[if_top];
    // CRITICAL FIX: இன்னர் பிளாக்குகளின் ஓவர்லேப் பக் தவிர்க்க உடனே ஸ்டேக்கை பாப் செய்கிறோம்
    if_top--;

    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);

    // True Block அல்லது எல்ஸ் பார்ட்டில் இருந்து எண்ட் பிளாக்கிற்கு முறையான ஜம்ப் லிங்க்
    if (current_bb && LLVMGetBasicBlockTerminator(current_bb) == NULL) {
        LLVMBuildBr(builder, ctx.end_block);
    }

    // கோடுல 'else' பிளாக் இல்லை என்றால், False Block-ஐ நேரடியாக End Block-உடன் இணைத்தல்
    if (!ctx.has_else) {
        LLVMPositionBuilderAtEnd(builder, ctx.false_block);
        if (LLVMGetBasicBlockTerminator(ctx.false_block) == NULL) {
            LLVMBuildBr(builder, ctx.end_block);
        }
    }

    // பில்டரை பாதுகாப்பாக அடுத்த கமாண்டுகளை ஏற்க 'if_end_X' பிளாக்கின் முடிவில் நிலைநிறுத்துதல்
    LLVMPositionBuilderAtEnd(builder, ctx.end_block);
}
