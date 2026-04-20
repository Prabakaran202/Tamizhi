#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
void skip_block(FILE *file); // இதை முன்னாடியே அறிவிக்கிறோம் (Prototype)


void parse(FILE *file) {
    Token t;
    fprintf(stderr, "[Parser] Tamizhi Engine Analysis Started...\n");

    while ((t = get_next_token(file)).type != T_EOF) {
        
        // 1. 'முதன்மை' கண்டறியப்பட்டால்
        if (t.type == T_MAIN) {
            fprintf(stderr, "[Parser] Main block detected.\n");
            // '{' வரும் வரை தேடு
            while ((t = get_next_token(file)).type != 22 && t.type != T_EOF); 
            skip_block(file); // '}' வரை தாண்டு
        }

        // 2. 'நிகழ்' கண்டறியப்பட்டால்
        else if (t.type == T_FUNC) {
            fprintf(stderr, "[Parser] Function definition detected.\n");
            // ';' அல்லது '{' வரும் வரை தாண்டு
            while ((t = get_next_token(file)).type != 17 && t.type != 22 && t.type != T_EOF);
            if (t.type == 22) skip_block(file);
        }

        // 3. 'இயக்கு' கண்டறியப்பட்டால் (இதுதான் ரொம்ப முக்கியம்)
        else if (t.type == T_CALL || strcmp(t.value, "இயக்கு") == 0) {
            fprintf(stderr, "[Parser] Starting Execution (இயக்கு)...\n");
            
            // 'add' அல்லது 'T_ID' வரும் வரை தேடு
            while ((t = get_next_token(file)).type != T_ID && t.type != 23 && t.type != T_EOF);
            
            if (t.type == T_ID) {
                // '10' (முதல் எண்) வரும் வரை தேடு
                while ((t = get_next_token(file)).type != T_NUM && t.type != 23 && t.type != T_EOF);
                
                if (t.type == T_NUM) {
                    int n1 = atoi(t.value); // 10
                    get_next_token(file);   // '+' குறியீட்டைத் தாண்டு
                    Token second = get_next_token(file); // 20
                    int n2 = atoi(second.value);

                    fprintf(stderr, "[Parser] Logic Found: %d + %d\n", n1, n2);
                    
                    // Backend-க்கு அனுப்பு
                    tamizhi_gen_add_and_print(n1, n2);
                }
            }
        }
    }
}
