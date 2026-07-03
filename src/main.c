#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codegen.h"

#define TAMIZHI_VERSION "v0.1.5"

// 🌟 1. குளோபல் Debug Mode Flag
int tamizhi_debug_mode = 0;

// 🌟 2. THE MASTER BRIDGE: CLI ஃபங்ஷனை (tamizhi_cli.c-ல் உள்ளது) இங்கே இணைக்கிறோம்!
extern int tamizhi_cli_main(int argc, char *argv[]);

void print_tamizhi_environment() {
    printf("\033[1;36m==================================================\033[0m\n");
    printf("\033[1;32m         --- தமிழி ரன்டைம் இன்ஜின் (%s) ---\033[0m\n", TAMIZHI_VERSION);
    printf("\033[1;36m==================================================\033[0m\n\n");
    printf("\033[1;35m[Usage]:\033[0m\n");
    printf("  • tamizhi init <project_name>  (புதிய ப்ராஜெக்ட் உருவாக்க)\n");
    printf("  • tamizhi run <file_name.tz>   (கோப்பை இயக்க)\n");
    printf("  • tamizhi <file_name.tz> --debug   (For internal logs)\n");
    printf("\033[1;36m--------------------------------------------------\033[0m\n\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_tamizhi_environment();
        return 0;
    }

    // =================================================================
    // 🚀 THE MASTER FIX: CLI கட்டளைகளை (init) முதலில் பிரித்து CLI எஞ்சினுக்கு அனுப்புதல்
    // =================================================================
    if (strcmp(argv[1], "init") == 0) {
        return tamizhi_cli_main(argc, argv); // CLI ஸ்கேஃபோல்டிங் (Scaffolding) வேலை செய்யும்
    }

    // =================================================================
    // ⚙️ கோர் கம்பைலர் லாஜிக் (Core Compiler Engine)
    // =================================================================
    char *target_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
            tamizhi_debug_mode = 1; 
        } else if (strcmp(argv[i], "version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("Tamizhi Compiler Version: \033[1;32m%s\033[0m\n", TAMIZHI_VERSION);
            return 0;
        } else if (strcmp(argv[i], "help") == 0 || strcmp(argv[i], "--help") == 0) {
            print_tamizhi_environment();
            return 0;
        } 
        // "run" என்று டைப் செய்தால், அதை ஸ்கிப் செய்துவிட்டு அடுத்த ஃபைல் பெயரை எடுத்துக்கொள்ளும்!
        else if (strcmp(argv[i], "run") != 0) {
            target_file = argv[i];
        }
    }

    if (!target_file) {
        fprintf(stderr, "\033[1;31mபிழை: கோப்புப் பெயரை உள்ளிடவும்!\033[0m\n");
        return 1;
    }

    if (tamizhi_debug_mode) {
        fprintf(stderr, " \033[1;33m[System] Cleaning old artifacts before execution...\033[0m\n");
    }

    system("rm -f storage/output.bc storage/output.o storage/output.ll 2>/dev/null");
    system("rm -f storage/project_binary.dna 2>/dev/null");

    FILE *file = fopen(target_file, "r");
    if (!file) {
        fprintf(stderr, "\033[1;31mகோப்பைத் திறக்க முடியவில்லை: %s\033[0m\n", target_file);
        return 1;
    }

    tamizhi_codegen_init();
    tamizhi_generate_entry();

    if (tamizhi_debug_mode) {
        fprintf(stderr, "\033[1;32m--- தமிழி கம்பைலர் (%s) [DEBUG MODE] ---\033[0m\n", TAMIZHI_VERSION);
    }

    parse(file); 
    tamizhi_codegen_finish();
    fclose(file);

    if (tamizhi_debug_mode) {
        fprintf(stderr, "\n\033[1;36mதொகுப்பு மற்றும் ஆய்வு வெற்றிகரமாக முடிந்தது.\033[0m\n");
    }

    return 0;
}
