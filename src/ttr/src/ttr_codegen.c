#include <stdio.h>
#include "../core/ttr_codegen.h"

void ttr_generate_tamizhi_code(TTR_ASTProgram program, const char *output_filename) {
    FILE *fout = fopen(output_filename, "w");
    if (!fout) return;

    fprintf(fout, "main {\n");

    // பதிவான அத்தனை வரிகளையும் (Array) வரிசையாக தமிழிக்கு மாற்று!
    for (int i = 0; i < program.count; i++) {
        TTR_ASTNode node = program.nodes[i]; // 🔥 இங்கே TTR_ASTNode என மாற்றப்பட்டுள்ளது
        
        if (node.type == AST_PRINT) {
            fprintf(fout, "    print(\"%s\");\n", node.value);
        } 
        else if (node.type == AST_PRINT_VAR) {
            // நீங்க nano-ல காட்டிய அதே சிண்டாக்ஸ்!
            fprintf(fout, "    print(%s);\n", node.var_name); 
        } 
        else if (node.type == AST_ASSIGNMENT) {
            fprintf(fout, "    %s %s = %s ;\n", node.var_type, node.var_name, node.value);
        }
    }

    fprintf(fout, "}\n");
    fprintf(fout, "\nfooter {\n    // End of program\n}\n");

    fclose(fout);
    printf("✅ மாஸ்! பைத்தான் கோட் வெற்றிகரமாக தமிழிக்கு மாற்றப்பட்டது: %s\n", output_filename);
}
