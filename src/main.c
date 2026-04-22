#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "codegen.h"

int main(int argc, char *argv[]) {
    // 1. பயன்பாட்டு முறை சரிபார்ப்பு
    if (argc < 2) {
        fprintf(stderr, "பயன்பாடு: tamizhi <filename.tz>\n");
        return 1;
    }

    // 2. தமிழி கோப்பை திறத்தல்
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("கோப்பை திறக்க முடியவில்லை");
        return 1;
    }

    // 3. LLVM Backend Engine தயார் செய்தல்
    tamizhi_codegen_init();
    tamizhi_generate_entry();

    fprintf(stderr, "--- தமிழி கம்பைலர் (v0.1) ---\n");

    // 4. Parser-ஐ இயக்குதல் (இதுதான் லெக்சர் மற்றும் கோட்ஜென்-ஐ இணைக்கும்)
    parse(file); 

    // 5. LLVM IR கோடை உருவாக்கி அவுட்புட் செய்தல்
    // இந்த பங்க்ஷன் codegen.c-ல் உள்ளது, அங்கிருந்து இதைக் கூப்பிடுகிறோம்.
    tamizhi_codegen_finish();

    // 6. கோப்பை மூடுதல்
    fclose(file);
    fprintf(stderr, "\nதொகுப்பு மற்றும் ஆய்வு முடிந்தது.\n");
    
    return 0;
}
