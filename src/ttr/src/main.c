#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/lexer.h"
#include "../core/parser.h"
#include "../core/codegen.h"

char* read_file(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: '%s' ஃபைலை திறக்க முடியவில்லை!\n", filename);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./ttr <filename.py>\n");
        return 1;
    }

    const char *input_file = argv[1];
    char output_file[256];
    
    snprintf(output_file, sizeof(output_file), "%s", input_file);
    char *dot = strstr(output_file, ".py");
    if (dot) strcpy(dot, ".tz");
    else strcat(output_file, ".tz");

    char *source_code = read_file(input_file);

    init_lexer(source_code);
    ASTProgram parsed_program = parse();

    if (parsed_program.count > 0) {
        generate_tamizhi_code(parsed_program, output_file);
    } else {
        printf("❌ Transpilation தோல்வி! பைத்தான் கோடில் பிழை உள்ளது.\n");
    }

    free(source_code);
    return 0;
}
