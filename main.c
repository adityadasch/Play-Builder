#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int main(int argc, char *argv[]){
    #ifdef BINNED
        char* OUTDIR =  ".\\bin\\";
    #else
        char* OUTDIR = ".\\";
    #endif
    if (argc>3) {
        printf("Expected upto 2 but got %d instead", argc-1);
        return 1;
    }
    if (argc==1){
        printf("Missing file name argument");
        return 1;
    }


    char* filename = argv[1];
    if (strcmp(filename, "--help")==0){
    	invoke_help();
	return 0;
    }
    if (strchr(filename, '.') == NULL){
        // Run filename
        char buf[100];
        sprintf(buf, "start cmd.exe /k \"(call %s\\%s&echo(&&echo Press any key to exit...&&pause >nul&&exit)\"", OUTDIR, filename);
        putdebug("Command: %s", buf);
        system(buf);
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

    while (*source == '/'||*source == ' '||*source == '\t') source++;
    // starts at text
    //printf("Breakwhere?");
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
    //system("start cmd.exe /c mkdir bin");
    system("IF NOT EXIST bin (mkdir bin)");
    system(buffer);

    return 0;
}
