// #include "lib/json.hpp"
#include <iostream>
#include <cstring>
#include "common.hpp"
#include "help.h"

int runFromConfig(void);
int runFromSource(void);
int create_config(void);
int edit_active(std::string& toName);
int erase_name(std::string& toName);
int list(void);

void invoke_help(){
    // Run xxd -i help.txt > help.h
    std::cout.write(
        reinterpret_cast<const char*>(help_txt),
        help_txt_len
    );
}

int main(int argc, char* argv[]){
    int err = 0;
    if (argc == 1)
        err = runFromConfig();
    else if (argc > 1 && std::strncmp(argv[1], "new", 3)==0){
        if (std::filesystem::exists("./play.json")){
            std::cout << "Config file exists" << std::endl;
            return 0;
        } 
        create_config();
        std::cout << "Config created" << std::endl;
        
    } else if (argc > 1 && std::strncmp(argv[1], "-set:", 5)==0 && std::strlen(argv[1]) > 5){
        std::string dt(argv[1]);
        dt = dt.substr(5);
        err = edit_active(dt);
        if (!err) std::cout << "Set active to " << dt << std::endl;
    } else if (argc > 1 && std::strncmp(argv[1], "-drop:", 6)==0 && std::strlen(argv[1]) > 6){
        std::string dt(argv[1]);
        dt = dt.substr(6);
        err = erase_name(dt);
        if (!err) std::cout << "Dropped " << dt << std::endl;
    } else if (argc > 1 && std::strncmp(argv[1], "-list", 5)==0 && std::strlen(argv[1]) == 5){
        err = list();
    } else if (argc > 1 && std::strncmp(argv[1], "-help", 5)==0 && std::strlen(argv[1]) == 5){
        invoke_help();
    } else std::cout << "Command not recognised. Try play --help" << std::endl;

        
    std::cout << "Exit with code "<< err << std::endl;
}