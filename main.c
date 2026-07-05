#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lib/cJSON.h"

#ifdef DEBUG
#include <stdarg.h>
#endif

void putdebug(const char *fmt, ...) {
    #ifdef DEBUG
    va_list args;
    va_start(args, fmt);

    printf("\n[DEBUG] ");

    vprintf(fmt, args);

    va_end(args);
    #endif
}

int parse_head_comment(char *source, char* dependency[], short depth){
    // a;b;c;d;f
    int item_count = 0;
    
    char *token = strtok(source, ";");

    while (token != NULL && item_count < depth) {
        dependency[item_count] = malloc(128*sizeof(char));
        strncpy(dependency[item_count], token, 127);

        dependency[item_count++][127] = '\0'; 
        token = strtok(NULL, ";");
    }
    return 0;
}

short count_segment(const char* source){
    // a;b -> 2 (No. of delim + 1)
    short delims;
    for (size_t i =0; i<strlen(source); i++){
        if (source[i] == ';') delims++;
    }
    return delims+1;
}

void invoke_help(){
	printf("Play Toolchain - Quick Compile & Run Utility\n"
"============================================\n"
"\n"
"USAGE:\n"
"    play <file.c> [<executabe_name>] -> Compiles file.c into configured folder\n"
"                                        with name provided or the name of the source file\n"
"    play <executable_name>           -> Searchs for executable by the name provided and runs it\n"
"    play --help                      -> Displays this screen\n"
"\n"
"CODE:\n"
"    The file passed as argument to Play should have the first line to be a \n"
"    comment with all the linking files mentioned without any extensions seperated\n"
"    by a semicolon(;).\n"
"    Eg: main.c\n"
"    // foo;bar;test\n"
"\n"
"DESCRIPTION:\n"
"    The Play Toolchain provides a lightweight way to compile and run C programs.\n"
"    It is designed for fast builds and simple execution without needing complex\n"
"    build systems or external scripts.\n"
"\n"
"FEATURES:\n"
"    - Compile source files quickly with a single command\n"
"    - Supports organized builds into ./bin when configured\n"
"    - Debug builds available with additional flags\n"
"    - Global access supported by adding the executables folder to PATH\n"
"\n"
"\n");
}

void filedrun(char* OUTDIR) {
    #define CLEAN do { \
        cJSON_Delete(json); \
        fclose(fp); \
        return; \
    } while(0)

    FILE *fp = fopen("play.json", "r");
    if (!fp) return;

    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = malloc(length + 1);
    if (!data) { fclose(fp); return; }
    fread(data, 1, length, fp);
    data[length] = '\0';

    cJSON *json = cJSON_Parse(data);
    free(data);
    if (!json) CLEAN;

    cJSON *entry = cJSON_GetObjectItemCaseSensitive(json, "entry");
    if (!entry || !cJSON_IsString(entry) || !entry->valuestring) CLEAN;

    char command[1024] = "gcc ";
    strncat(command, entry->valuestring, sizeof(command)-strlen(command)-1);

    cJSON *depends = cJSON_GetObjectItemCaseSensitive(json, "depends");
    if (depends && cJSON_IsArray(depends)) {
        cJSON *item = NULL;
        cJSON_ArrayForEach(item, depends) {
            if (cJSON_IsString(item) && item->valuestring) {
                strncat(command, " ", sizeof(command)-strlen(command)-1);
                strncat(command, item->valuestring, sizeof(command)-strlen(command)-1);
            } else {
                printf("Invalid depends entry\n");
                CLEAN;
            }
        }
    }

    cJSON *flags = cJSON_GetObjectItemCaseSensitive(json, "flags");
    if (flags && cJSON_IsArray(flags)) {
        cJSON *item = NULL;
        cJSON_ArrayForEach(item, flags) {
            if (cJSON_IsString(item) && item->valuestring) {
                strncat(command, " -", sizeof(command)-strlen(command)-1);
                strncat(command, item->valuestring, sizeof(command)-strlen(command)-1);
            } else {
                printf("Invalid flag entry\n");
                CLEAN;
            }
        }
    }

    char objname[256];
    cJSON *bin = cJSON_GetObjectItemCaseSensitive(json, "bin");
    if (bin && cJSON_IsString(bin) && bin->valuestring) {
        strncat(command, " -o ", sizeof(command)-strlen(command)-1);
        strncat(command, OUTDIR, sizeof(command)-strlen(command)-1);
        strncat(command, bin->valuestring, sizeof(command)-strlen(command)-1);
        strncpy(objname, bin->valuestring, sizeof(objname)-1);
        objname[sizeof(objname)-1] = '\0';
    } else {
        strncpy(objname, entry->valuestring, sizeof(objname)-1);
        objname[sizeof(objname)-1] = '\0';
        char *dot = strchr(objname, '.');
        if (dot) *dot = '\0';
        strncat(command, " -o ", sizeof(command)-strlen(command)-1);
        strncat(command, OUTDIR, sizeof(command)-strlen(command)-1);
        strncat(command, objname, sizeof(command)-strlen(command)-1);
    }

    putdebug("Command: %s\n", command);
    char buffer[1000];
    sprintf(buffer, "start cmd.exe /k \"%s&&(call %s%s&echo(&&echo Press any key to exit...&&pause >nul&&exit)", command,OUTDIR, objname);
    system(buffer);
    CLEAN;
}

void binrun(char* OUTDIR, char* filename){
    char buf[100];
    sprintf(buf, "start cmd.exe /k \"(call %s\\%s&echo(&&echo Press any key to exit...&&pause >nul&&exit)\"", OUTDIR, filename);
    putdebug("Command: %s", buf);
    system(buf);
}

void blankrun(char* OUTDIR, char* filename, char* objname){
    char template[512] = "gcc ";
    strcat(template, filename);

    strcat(template, " -o ");
    strcat(template, OUTDIR); 
    strcat(template, objname);

    char buffer[1000];
    sprintf(buffer, "start cmd.exe /k \"%s&&(call %s%s&echo(&&echo Press any key to exit...&&pause >nul&&exit)", template,OUTDIR, objname);
    putdebug("Command: %s", buffer);
    system(buffer);
}

int main(int argc, char *argv[]){
    #ifdef BINNED
        char* OUTDIR =  ".\\bin\\";
    	system("IF NOT EXIST bin (mkdir bin)");
    #else
        char* OUTDIR = ".\\";
    #endif
    if (argc>3) {
        printf("Expected upto 2 but got %d instead", argc-1);
        return 1;
    }
    if (argc==1){
        filedrun(OUTDIR);
        return 0;
    }


    char* filename = argv[1];
    if (strcmp(filename, "--help")==0){
    	invoke_help();
	return 0;
    }
    if (strchr(filename, '.') == NULL){
        // Run filename
        binrun(OUTDIR, filename);
        return 0;
    }

    char objname[256];

    if (argc == 3) {
        strncpy(objname, argv[2], sizeof(objname)-1);
        objname[sizeof(objname)-1] = '\0';
    }
    else{
        strncpy(objname, filename, sizeof(objname)-1);
        objname[sizeof(objname)-1] = '\0';
        *strchr(objname, '.') = '\0';
        putdebug("OBJNAME: %s\n", objname);
    }
    putdebug("FILENAME: %s", filename);
    
    FILE* file;
    file = fopen(filename, "r");

    if (file == NULL) {
        printf("File couldn't be found");
        return 1;
    }

    char text[2048];
    fgets(text, 2048, file);

    char *newline = strchr(text, '\n');  // simpler than strstr
    if (newline != NULL) {
        *newline = '\0';
    }

    putdebug(" String len: %d", strlen(text));
    char* source = text;

    if (*source!='/') {
        printf("Do this");
        blankrun(OUTDIR, filename, objname);
        fclose(file);
        return 0;
    }

    while (*source == '/'||*source == ' '||*source == '\t') source++;
    // starts at text
    short depth = count_segment(source);
    char *dependency[depth];
    
    parse_head_comment(source, dependency, depth);

    char template[512] = "gcc ";
    strcat(template, filename);

    for (int i = 0; i<depth;i++){
        strcat(template, " ");
        strcat(template, dependency[i]);
        strcat(template, ".c");
    }

    fclose(file);

    strcat(template, " -o ");
    strcat(template, OUTDIR); 
    strcat(template, objname);

    char buffer[1000];
    sprintf(buffer, "start cmd.exe /k \"%s&&(call %s%s&echo(&&echo Press any key to exit...&&pause >nul&&exit)", template,OUTDIR, objname);
    putdebug("Command: %s", buffer);
    system(buffer);

    return 0;
}
