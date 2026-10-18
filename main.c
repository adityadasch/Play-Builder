#include "main.h"

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
    return 0;}

short count_segment(const char* source){
    // a;b -> 2 (No. of delim + 1)
    short delims;
    for (size_t i =0; i<strlen(source); i++){
        if (source[i] == ';') delims++;
    }
    return delims+1;}

void invoke_help(){
    printf("Play Toolchain - Quick Compile & Run Utility\n"
    "============================================\n"
    "\n"
    "USAGE:\n"
    "    play <file.c> [<executabe_name>] -> Compiles file.c into configured folder\n"
    "                                        with name provided or the name of the source file\n"
    "    play <executable_name>           -> Searchs for executable by the name provided and runs it\n"
    "    play --help                      -> Displays this screen\n"
    "    play -suite                      -> Changes active suite\n"
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
    "\n");}

void read_json(FILE **fp, cJSON **json){
    if (json==nullptr) return;

    *fp = fopen("play.json", "r");
    if (!*fp) return;

    fseek(*fp, 0, SEEK_END);
    long length = ftell(*fp);
    fseek(*fp, 0, SEEK_SET);

    char *data = malloc(length + 1);
    if (!data) { fclose(*fp); return; }
    fread(data, 1, length, *fp);
    data[length] = '\0';

    *json = cJSON_Parse(data);
    free(data);
}

void change_active(char* filename){
    #define CLEAN do { \
        cJSON_Delete(json); \
        fclose(fp); \
        return; \
    } while(0)

    FILE *fp; cJSON *json;
    read_json(&fp, &json);
    if (!json) CLEAN;

    cJSON *active = cJSON_GetObjectItemCaseSensitive(json, "active");
    if (!active || !cJSON_IsString(active) || !active->valuestring) CLEAN;
    free(active->valuestring);
    active->valuestring = filename;
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        cJSON_Delete(json);
        return;
    }
    FILE *file = fopen("play.json", "w");
    if (file != NULL) {
        fputs(json_string, file);
        fclose(file);
        printf("Active has been changed");
    } else {
        printf("Failed to open file.\n");
    }
}

void filedrun(char* OUTDIR) {
    #define CLEAN do { \
        cJSON_Delete(json); \
        fclose(fp); \
        return; \
    } while(0)

    FILE *fp; cJSON *json;
    read_json(&fp, &json);
    if (!json) CLEAN;

    cJSON *active = cJSON_GetObjectItemCaseSensitive(json, "active");
    if (!active || !cJSON_IsString(active) || !active->valuestring) CLEAN;

    cJSON *json_subset = cJSON_GetObjectItemCaseSensitive(json, active->valuestring);
    if (!json_subset || !cJSON_IsObject(json_subset)) CLEAN;

    cJSON *entry = cJSON_GetObjectItemCaseSensitive(json_subset, "entry");
    if (!entry || !cJSON_IsString(entry) || !entry->valuestring) CLEAN;

    char command[1024] = "gcc ";
    strncat(command, entry->valuestring, sizeof(command)-strlen(command)-1);

    cJSON *depends = cJSON_GetObjectItemCaseSensitive(json_subset, "depends");
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

    cJSON *flags = cJSON_GetObjectItemCaseSensitive(json_subset, "flags");
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
    cJSON *bin = cJSON_GetObjectItemCaseSensitive(json_subset, "bin");
    if (bin && cJSON_IsString(bin) && bin->valuestring) {
        strncat(command, " -o ", sizeof(command)-strlen(command)-1);
        strncat(command, OUTDIR, sizeof(command)-strlen(command)-1);
        strncat(command, bin->valuestring, sizeof(command)-strlen(command)-1);
        strncat(command, EXEEXT, 5);
        strncpy(objname, bin->valuestring, sizeof(objname)-1);
        strncat(objname, EXEEXT, 5);
        putdebug("exec:%s\n", objname);
        
        objname[sizeof(objname)-1] = '\0';
    } else {
        strncpy(objname, entry->valuestring, sizeof(objname)-1);
        strncpy(objname, EXEEXT, sizeof(objname)-1);
        objname[sizeof(objname)-1] = '\0';
        strncat(command, " -o ", sizeof(command)-strlen(command)-1);
        strncat(command, OUTDIR, sizeof(command)-strlen(command)-1);
        strncat(command, objname, sizeof(command)-strlen(command)-1);
    }

    putdebug("Command: %s\n", command);
    char buffer[BUFFERMAGICNUMBER];
    sprintf(buffer, FILEDRUNCMD, command,OUTDIR, objname);
    system(buffer);
    CLEAN;
}

void binrun(char* OUTDIR, char* filename){
    char buf[100];
    sprintf(buf, BINRUNCMD, OUTDIR, filename);
    putdebug("Command: %s", buf);
    system(buf);
}

void blankrun(char* OUTDIR, char* filename, char* objname){
    char template[512] = "gcc ";
    strcat(template, filename);

    strcat(template, " -o ");
    strcat(template, OUTDIR); 
    strcat(template, objname);
    strcat(template, EXEEXT);

    char objbuf[100];
    sprintf(objbuf, "%s%s", objname, EXEEXT);

    char buffer[BUFFERMAGICNUMBER];
    sprintf(buffer, FILEDRUNCMD, template,OUTDIR, objbuf);
    putdebug("Command: %s", buffer);
    system(buffer);
}

bool checkjson(){
    FILE* fp = fopen("./play.json", "r");
    if (fp != nullptr){
        fclose(fp);
        return true;
    }
    return false;
}

int main(int argc, char *argv[]){
    #ifdef BINNED
        #if defined(_WIN32) || defined(_WIN64)
            char* OUTDIR =  ".\\bin\\";
    	    system("IF NOT EXIST bin (mkdir bin)");
        #elif defined(__linux__)
            char* OUTDIR =  "./bin/";
    	    system("mkdir -p bin");
        #endif
    #else
        #if defined(_WIN32) || defined(_WIN64)
            char* OUTDIR = ".\\";
        #elif defined(__linux__)
            char* OUTDIR = "./";
        #endif
    #endif
    if (argc>3) {
        printf("Expected upto 2 but got %d instead", argc-1);
        return 1;
    }
    if (argc==1){
        if (checkjson()){
            filedrun(OUTDIR);
        } else {
            FILE* fp = fopen("play.json","w");
            if (fp != nullptr){
                fprintf(fp, "{\n    \"active\":\"main\",\n    \"main\":\n    {\n        \"entry\": \"main.c\",\n        \"depends\": [],\n        \"flags\": [],\n        \"bin\": \"output_file_name\"\n    }\n}");
                printf("Play.json created! Go and configure it!\n");
                fclose(fp);
            }
        }
        return 0;
    }


    char* filename = argv[1];
    if (strcmp(filename, "--help")==0){
    	invoke_help();
	return 0;
    }
    if (filename[0] == '-'){
    	change_active(++filename);
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
    strcat(template, EXEEXT);

    char objbuf[300];
    sprintf(objbuf, "%s%s", objname, EXEEXT);    

    char buffer[BUFFERMAGICNUMBER];
    sprintf(buffer, FILEDRUNCMD, template,OUTDIR, objbuf);
    putdebug("Command: %s", buffer);
    system(buffer);

    return 0;
}
