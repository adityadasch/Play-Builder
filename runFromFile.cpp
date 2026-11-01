#include "common.hpp"

void parse_file(std::string entry_file, std::vector<std::string>& deps){
    std::ifstream i(entry_file);
    std::string line;
    std::getline(i, line);

    if (line.find_first_not_of("dep:") == std::string::npos) return 0;

    line.replace(0, line.find_first_of(":")+1, "");

    std::istringstream ls(line);
    std::string op;
    while(std::getline(ls, op, ';')){
        deps.push_back(op);
    }
}

int extract_dependecies(std::string entry_file, Config& config){
    config = {};
    config.entry = entry_file;
    size_t index_dot = entry_file.find_last_of('.');
    if(index_dot == std::string::npos){
        PUT << "Sorry, neither is the lang config-ed nor the file has extension" << std::endl;
        return 1;
    }
    config.bin = entry_file.substr(0,index_dot);
    std::string ext = entry_file.substr(index_dot+1);
    config.compiler = (ext == "c") ? "gcc" : ((ext == "cpp") ? "g++" : "null");
    if (config.compiler == "null") {
        PUT << "The file extension of entry-file is not supported" << std::endl;
        return 1;
    }
    
    // config = {compiler, entry_file, filename, {}, {}, {}, false};
    return 0;
}

void convert_config(Config* config, std::vector<char*>& args, std::vector<std::string>& storage){
    storage.reserve(4+config->dependencies.size()); // g** entry -o bin -depends nullptr 
    storage.push_back(config->compiler);
    storage.push_back(config->entry);
    storage.push_back("-o");
    storage.push_back(config->bin);
    for(std::string sp: config->dependencies){
        storage.push_back(sp);
    }
    
    args.reserve(storage.size() + 1);
    for(std::string& val: storage){
        args.push_back(val.data());
    } 

    args.push_back(nullptr);
}

int runFromSource(std::string entry_file, std::string& bin, bool& compFail){
    Config config;
    int err = 0;
    err = extract_dependecies(entry_file, config);
    if (err) return err;
    parse_file(entry_file, config.dependencies);
    std::vector<char*> args; std::vector<std::string> storage;
    convert_config(&config, args, storage);
    err = runProcess(config.compiler.data(), args);
    
    if (err){
        PUT << "Something went wrong during compilation" << std::endl;
        compFail = true;
        return 5;
    }

    err = runProcess(args.at(3), {});
    bin = std::string(args.at(3));
    return err;
}