#include <iostream>
#include <cstring>
#include "common.hpp"
#include "help.h"

int runFromConfig(std::string& bin, bool& compFail);
int runFromSource(std::string entry_file, std::string& bin, bool& compFail);
int create_config(void);
int edit_active(std::string& toName);
int erase_name(std::string& toName);
int list(void);

void invoke_help(){
    // Run xxd -i help.txt > help.h
    PUT.write(
        reinterpret_cast<const char*>(help_txt),
        help_txt_len
    );
    std::cout << std::endl;
}

int main(int argc, char* argv[]){
    int err = 0; bool compFail = false;
    std::string bin("");
    if (argc == 1){
        // Default is run from config
        if (std::filesystem::exists("./play.json")) err = runFromConfig(bin, compFail);
        else PUT << "Play.json file doesn't exist. \n\tCreate config file or run `play new` to create template file" << std::endl;
    }
    else if (argc > 1 && std::strncmp(argv[1], "new", 3)==0){
        if (std::filesystem::exists("./play.json")){
            PUT << "Config file already exists. Exiting." << std::endl;
            return 0;
        } 
        create_config();
        PUT << "Config created in " << std::filesystem::current_path() << std::endl;
        
    } else if (argc > 1 && std::strncmp(argv[1], "-set:", 5)==0 && std::strlen(argv[1]) > 5){
        std::string dt(argv[1]);
        dt = dt.substr(5);
        err = edit_active(dt);
        if (!err) PUT << "Set active to " << dt << std::endl;
    } else if (argc > 1 && std::strncmp(argv[1], "-drop:", 6)==0 && std::strlen(argv[1]) > 6){
        std::string dt(argv[1]);
        dt = dt.substr(6); // No error check, if statement handles that
        err = erase_name(dt);
        if (!err) PUT << "Dropped " << dt << std::endl;
    } else if (argc > 1 && std::strncmp(argv[1], "-list", 5)==0 && std::strlen(argv[1]) == 5){
        err = list();
    } else if (argc > 1 && std::strncmp(argv[1], "-help", 5)==0 && std::strlen(argv[1]) == 5){
        invoke_help();
    } else if (argc > 1 && argv[1][0]!='-') err = runFromSource(std::string(argv[1]), bin, compFail); 
    else PUT << "Command not recognised. Try play -help" << std::endl;
        
    std::cout << "Exit with code "<< err << std::endl;
    if (!bin.empty() && !compFail) std::cout << "Binary compiled to " << bin << std::endl;
}