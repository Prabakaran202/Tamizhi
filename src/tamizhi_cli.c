#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_banner() {
    printf("----------------------------------------\n");
    printf("   தமிழி (Tamizhi) Compiler V0.1       \n");
    printf("   Open Source @ Backend Developer Hub \n");
    printf("   Author: Prabakaran                  \n");
    printf("----------------------------------------\n\n");
}

int tamizhi_cli_main(int argc, char *argv[]) {
    // 🌟 1. குறைவான ஆர்கியுமெண்ட் வந்தால் Help Menu காட்டவும்
    if (argc < 2) {
        print_banner();
        printf("பயன்பாடு (Usage):\n");
        printf("  tamizhi run <filename.tz>   -> கோப்பை இயக்க\n");
        printf("  tamizhi init <project>      -> புதிய ப்ராஜெக்ட் உருவாக்க\n");
        return 1;
    }

    char *command = argv[1];

    // ========================================================
    // 🚀 PHASE 1: INIT COMMAND (Virtual Env & Scaffolding)
    // ========================================================
    if (strcmp(command, "init") == 0) {
        if (argc < 3) {
            printf("தவறு: ப்ராஜெக்ட் பெயர் கொடுக்கப்படவில்லை!\n");
            printf("பயன்பாடு: tamizhi init <project_name>\n");
            return 1;
        }
        
        char *project_name = argv[2];
        char cmd[1024];

        printf("[1/3] '%s' ஃபோல்டர் ஸ்ட்ரக்சர் உருவாக்கப்படுகிறது...\n", project_name);
        
        // ஃபோல்டர்களை உருவாக்குதல்
        sprintf(cmd, "mkdir -p %s/.tamizhi-env/packages", project_name);
        system(cmd);
        sprintf(cmd, "mkdir -p %s/.tamizhi-env/bin", project_name);
        system(cmd);
        sprintf(cmd, "mkdir -p %s/src", project_name);
        system(cmd);

        printf("[2/3] Config (tamizhi.json) உருவாக்கப்படுகிறது...\n");
        sprintf(cmd, "%s/tamizhi.json", project_name);
        FILE *config = fopen(cmd, "w");
        if (config) {
            fprintf(config, "{\n");
            fprintf(config, "  \"name\": \"%s\",\n", project_name);
            fprintf(config, "  \"version\": \"1.0.0\",\n");
            fprintf(config, "  \"description\": \"Tamizhi Standard Project\",\n");
            fprintf(config, "  \"dependencies\": {}\n");
            fprintf(config, "}\n");
            fclose(config);
        }

        printf("[3/3] முதல் கோப்பு (src/main.tz) உருவாக்கப்படுகிறது...\n");
        sprintf(cmd, "%s/src/main.tz", project_name);
        FILE *main_file = fopen(cmd, "w");
        if (main_file) {
            fprintf(main_file, "main {\n");
            fprintf(main_file, "    print \"வணக்கம்! %s ப்ராஜெக்ட் வெற்றிகரமாக இயங்குகிறது!\";\n", project_name);
            fprintf(main_file, "}\n");
            fclose(main_file);
        }

        printf("\n🎉 வெற்றி! '%s' ப்ராஜெக்ட் தயாராகிவிட்டது.\n", project_name);
        printf("இப்போது இயக்குவதற்கு:\n");
        printf("👉 cd %s\n", project_name);
        printf("👉 tamizhi run src/main.tz\n");

    } 
    // ========================================================
    // 🚀 PHASE 2: RUN COMMAND (Compile & Execute)
    // ========================================================
    else if (strcmp(command, "run") == 0) {
        if (argc < 3) {
            printf("தவறு: கோப்பு பெயர் கொடுக்கப்படவில்லை!\n");
            printf("பயன்பாடு: tamizhi run <filename.tz>\n");
            return 1;
        }

        char *filename = argv[2];

        // 🌟 2. RUN கமாண்டில் மட்டும் கோப்பு இருக்கிறதா என சோதிக்கவும்
        if (access(filename, F_OK) == -1) {
            printf("தவறு: '%s' என்ற கோப்பு காணப்படவில்லை!\n", filename);
            return 1;
        }

        char base_name[256];
        strcpy(base_name, filename);
        char *dot = strrchr(base_name, '.');
        if (dot) *dot = '\0'; 

        char cmd[1024];

        printf("[1/3] தமிழி ஆய்வு செய்கிறது...\n");
        sprintf(cmd, "tamizhi_core %s > %s.ll", filename, base_name);

        if (system(cmd) != 0) {
            printf("தவறு: தமிழி கோப்பில் பிழை உள்ளது அல்லது 'tamizhi_core' நிறுவப்படவில்லை!\n");
            return 1;
        }

        printf("[2/3] மெஷின் கோடாக மாற்றுகிறது...\n");
        sprintf(cmd, "clang -x ir %s.ll -o %s_bin", base_name, base_name);
        if (system(cmd) != 0) {
            printf("தவறு: Clang மூலம் கம்பைல் செய்வதில் சிக்கல்!\n");
            return 1;
        }

        printf("[3/3] இயங்குகிறது:\n\n");
        sprintf(cmd, "./%s_bin", base_name);
        system(cmd);

        sprintf(cmd, "rm %s.ll", base_name);
        system(cmd);

    } 
    // ========================================================
    // ❌ ERROR HANDLING
    // ========================================================
    else {
        printf("தவறான கட்டளை: %s\n", command);
        printf("பயன்பாடு:\n  tamizhi run <file>\n  tamizhi init <project>\n");
    }

    return 0;
}
