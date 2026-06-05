#include "codegen_bridge.h"

extern void encode_logic(const char* input_path, const char* output_path);

void tamizhi_binary_to_dna_storage(const char* filename) {
    if (!filename) return;
    FILE *check = fopen(filename, "rb");
    if (check) {
        fclose(check);
        encode_logic(filename, "storage/project_binary.dna"); 
        fprintf(stderr, " [DNA-VM] Binary AOT Secured at: storage/project_binary.dna\n");
    }
}
