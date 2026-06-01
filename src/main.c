#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codegen.h"

// 🌟 வெர்ஷனை குளோபல் மேக்ரோவாக அறிவிக்கிறோம் (இனி இங்க மட்டும் மாத்தினால் போதும் பிரபா!)
#define TAMIZHI_VERSION "v0.0.1"

// 🌟 தமிழி மொழிக்கான பிரத்தியேக கலைநயமிக்க பேனர் மற்றும் சூழல் தகவல்
void print_tamizhi_environment() {
    printf("\033[1;36m==================================================\033[0m\n");
    printf("\033[1;33m  _____              _     _     _ \n");
    printf(" |_   _|_ _ _ __ ___ (_)___| |__ (_)\n");
    printf("   | |/ _` | '_ ` _ \\| |_  / '_ \\| |\n");
    printf("   | | (_| | | | | | | |/ /| | | | |\n");
    printf("   |_|\\__,_|_| |_| |_|_/___|_| |_|_|\033[0m\n");
    // 🌟 மேக்ரோ வெர்ஷனை இங்க டைனமிக்காக அப்ளை பண்றோம்
    printf("\033[1;32m         --- தமிழி ரன்டைம் இன்ஜின் (%s) ---\033[0m\n", TAMIZHI_VERSION);
    printf("\033[1;36m==================================================\033[0m\n\n");

    // 🚀 என்விரான்மென்ட் தகவல்கள் (Node.js ஸ்டைலில்)
    printf("\033[1;34m[Runtime Info]\033[0m\n");
    printf("  • Engine Status : \033[1;32mActive (LLVM JIT Enabled)\033[0m\n");
    printf("  • Core Support  : Native Tamil & English Keywords\033[0m\n");
    printf("  • Purpose       : High-Speed Linux System Automation\033[0m\n");
    printf("  • DNA Storage   : Enabled (storage/project_binary.dna)\033[0m\n");
    printf("  • Architecture  : Android Termux (aarch64-linux-android)\033[0m\n\n");
    
    printf("\033[1;35m[Usage]:\033[0m\n");
    printf("  • tamizhi run <file_name.tz>\n");
    printf("  • tamizhi <file_name.tz>\n");
    printf("\033[1;36m--------------------------------------------------\033[0m\n\n");
}

int main(int argc, char *argv[]) {
    // 1. பயன்பாட்டு முறை சரிபார்ப்பு (ஆர்குமெண்ட்ஸ் எதுவும் இல்லை என்றால் பேனரைக் காட்டு)
    if (argc < 2) {
        print_tamizhi_environment();
        return 0;
    }

    char *target_file;

    // 2. 'run' என்ற வார்த்தை இருந்தால் அடுத்த ಆர்குமெண்ட்டை கோப்பாக எடு
    if (strcmp(argv[1], "run") == 0) {
        if (argc < 3) {
            fprintf(stderr, "\033[1;31mபிழை: கோப்புப் பெயரை உள்ளிடவும்!\033[0m\n");
            printf("பயன்பாடு: tamizhi run <filename.tz>\n");
            return 1;
        }
        target_file = argv[2];
    } else {
        // 🌟 பயனர் 'version' அல்லது '-v' அல்லது '--version' என்று கொடுத்தால் வெர்ஷனை மட்டும் காட்டுறோம்
        if (strcmp(argv[1], "version") == 0 || strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            printf("Tamizhi Compiler Version: \033[1;32m%s\033[0m\n", TAMIZHI_VERSION);
            return 0;
        }
        // ஒருவேளை 'help' அல்லது '--help' என்று கொடுத்தால் பேனரைக் காட்டு
        if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0) {
            print_tamizhi_environment();
            return 0;
        }
        target_file = argv[1];
    }

    // 3. தமிழி கோப்பைத் திறத்தல்
    FILE *file = fopen(target_file, "r");
    if (!file) {
        fprintf(stderr, "\033[1;31mகோப்பைத் திறக்க முடியவில்லை!\033[0m\n");
        fprintf(stderr, "தேடப்பட்ட கோப்பு: %s\n", target_file);
        return 1;
    }

    // 4. LLVM Backend Engine தயார் செய்தல்
    tamizhi_codegen_init();
    tamizhi_generate_entry();

    // 🌟 இங்கேயும் மேக்ரோ வெர்ஷனை டைனமிக்கா பிரிண்ட் பண்றோம்
    fprintf(stderr, "\033[1;32m--- தமிழி கம்பைலர் (%s) ---\033[0m\n", TAMIZHI_VERSION);

    // 5. Parser-ஐ இயக்குதல்
    parse(file); 

    // 6. LLVM IR மற்றும் DNA-VM பணிகளை முடித்தல்
    tamizhi_codegen_finish();

    // 7. கோப்பை மூடுதல்
    fclose(file);
    fprintf(stderr, "\n\033[1;36mதொகுப்பு மற்றும் ஆய்வு வெற்றிகரமாக முடிந்தது.\033[0m\n");

    return 0;
}
