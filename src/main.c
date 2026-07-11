#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"
#include "codegen.h"

#define TAMIZHI_VERSION "v2.1.4"

int tamizhi_debug_mode = 0;
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

    if (strcmp(argv[1], "init") == 0) {
        return tamizhi_cli_main(argc, argv);
    }

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
        } else if (strcmp(argv[i], "run") != 0) {
            target_file = argv[i];
        }
    }

    if (!target_file) {
        fprintf(stderr, "\033[1;31mபிழை: கோப்புப் பெயரை உள்ளிடவும்!\033[0m\n");
        return 1;
    }

    // கோப்பு இருக்கிறதா என சரிபார்க்க
    if (access(target_file, F_OK) == -1) {
        fprintf(stderr, "\033[1;31mதவறு: '%s' என்ற கோப்பு காணப்படவில்லை!\033[0m\n", target_file);
        return 1;
    }

    if (tamizhi_debug_mode) {
        fprintf(stderr, " \033[1;33m[System] Preparing environment...\033[0m\n");
    }

    // 🌟 1. Environment & Storage செட்-அப்
    system("mkdir -p storage 2>/dev/null");
    system("rm -f storage/output.bc storage/output.o storage/output.ll 2>/dev/null");
    system("rm -f storage/project_binary 2>/dev/null");

    FILE *file = fopen(target_file, "r");
    if (!file) {
        fprintf(stderr, "\033[1;31mகோப்பைத் திறக்க முடியவில்லை: %s\033[0m\n", target_file);
        return 1;
    }

    // 🌟 2. தமிழி Parsing மற்றும் LLVM IR உருவாக்கம்
    tamizhi_codegen_init();
    tamizhi_generate_entry();

    if (tamizhi_debug_mode) {
        fprintf(stderr, "\033[1;32m--- தமிழி கம்பைலர் (%s) [DEBUG MODE] ---\033[0m\n", TAMIZHI_VERSION);
    }

    parse(file); 
    
    // 🌟 இந்த ஃபங்ஷன் இப்போது JIT-ஐ இயக்காமல் 'storage/output.ll' ஃபைலை மட்டும் உருவாக்கும்
    tamizhi_codegen_finish();
    fclose(file);

    if (tamizhi_debug_mode) {
        fprintf(stderr, "\n\033[1;36mதொகுப்பு (Parsing) வெற்றிகரமாக முடிந்தது.\033[0m\n");
    }

    // ==========================================================
    // 🚀 THE MAGIC: Clang Linking & Execution Phase (Dynamic Path Fix)
    // ==========================================================
    printf("\033[1;34m[Building] Linking Native C Runtime...\033[0m\n");

    char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "\033[1;31mதவறு: HOME environment variable இல்லை!\033[0m\n");
        return 1;
    }

    char pip_path[512];
    char git_path[512];

    // Pip மற்றும் GitHub இரண்டின் முழுப் பாதைகளையும் உருவாக்குதல்
    sprintf(pip_path, "%s/tamizhi-extract/core/http_runtime.c", home);
    sprintf(git_path, "%s/Tamizhi/core/http_runtime.c", home);

    char *final_runtime_path = NULL;

    // எங்கு ஃபைல் இருக்கிறது என்பதை உறுதிப்படுத்துதல்
    if (access(pip_path, F_OK) == 0) {
        final_runtime_path = pip_path;
    } else if (access(git_path, F_OK) == 0) {
        final_runtime_path = git_path;
    } else {
        fprintf(stderr, "\033[1;31mதவறு: 'http_runtime.c' ரன்டைம் ஃபைல் காணப்படவில்லை!\033[0m\n");
        fprintf(stderr, "தேடிய இடங்கள்:\n 1. %s\n 2. %s\n", pip_path, git_path);
        return 1;
    }

    // கண்டுபிடிக்கப்பட்ட சரியான பாதையை வைத்து Clang கமாண்டை உருவாக்குதல்
    char compile_cmd[1024];
    sprintf(compile_cmd, "clang -Wno-override-module storage/output.ll \"%s\" -o storage/project_binary", final_runtime_path);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "\033[1;31m\n[பிழை] கம்பைல் செய்வதில் (Clang) சிக்கல் ஏற்பட்டது!\033[0m\n");
        return 1;
    }

    printf("\033[1;32m[Success] Server App Ready! Running now...\033[0m\n\n");

    // உருவாக்கப்பட்ட நேட்டிவ் பைனரியை இயக்குகிறோம்!
    system("./storage/project_binary");

    return 0;
}
