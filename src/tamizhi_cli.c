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
    if (argc < 3) {
        print_banner();
        printf("பயன்பாடு: tamizhi run <filename.tz>\n");
        return 1;
    }

    char *command = argv[1];
    char *filename = argv[2];

    // கோப்பு இருக்கிறதா என்று ஒரு சோதனை
    if (access(filename, F_OK) == -1) {
        printf("தவறு: '%s' என்ற கோப்பு காணப்படவில்லை!\n", filename);
        return 1;
    }

    if (strcmp(command, "run") == 0) {
        char base_name[256];
        strcpy(base_name, filename);
        char *dot = strrchr(base_name, '.');
        if (dot) *dot = '\0'; 

        char cmd[1024];

        // 1. LLVM IR உருவாக்குதல்
        // இங்கதான் அந்த முக்கிய மாற்றம்: './' நீக்கப்பட்டுள்ளது
        printf("[1/3] தமிழி ஆய்வு செய்கிறது...\n");
        sprintf(cmd, "tamizhi_core %s > %s.ll", filename, base_name);
        
        if (system(cmd) != 0) {
            printf("தவறு: தமிழி கோப்பில் பிழை உள்ளது அல்லது 'tamizhi_core' நிறுவப்படவில்லை!\n");
            return 1;
        }

        // 2. Clang மூலம் பைனரி உருவாக்குதல்
        printf("[2/3] மெஷின் கோடாக மாற்றுகிறது...\n");
        sprintf(cmd, "clang -x ir %s.ll -o %s_bin", base_name, base_name);
        if (system(cmd) != 0) {
            printf("தவறு: Clang மூலம் கம்பைல் செய்வதில் சிக்கல்!\n");
            return 1;
        }

        // 3. பைனரியை இயக்குதல்
        printf("[3/3] இயங்குகிறது:\n\n");
        sprintf(cmd, "./%s_bin", base_name);
        system(cmd);

        // தற்காலிக .ll கோப்பை மட்டும் நீக்கலாம் (Optional)
        sprintf(cmd, "rm %s.ll", base_name);
        system(cmd);
        
    } else {
        printf("தவறான கட்டளை: %s\n", command);
    }

    return 0;
}
