#include "lib/json.hpp"
#include "common.hpp"

using json= nlohmann::ordered_json;

// TODO:
// Make sure file is ran only if compilation finishes properly. Done

void create_config(){
    std::ofstream o("play.json");

    json data = json::parse(R"({"_active":"main","_lang": "c","_compileToCwd": false,"main":{"entry": "main.c","depends": [],"flags": [],"bin": "main.out","args": []}})");

    o << std::setw(4) << data << std::endl;

    o.close();
}

int edit_active(std::string& toName){
    std::ifstream i("play.json");
    if (!i.is_open()){
        PUT << "Error while opening: play.json" << std::endl;
        if (i.fail()) PUT << "Badbit was set" << std::endl;
        if (i.fail()) PUT << "Error details: " << strerror(errno) << std::endl;
        return 1;
    }
    json data = json::parse(i);
    data["_active"] = toName;

    if (!data.contains(toName))
    {   
        // Active suite doesn't exit.
        std::string t;
        std::cout << "The config \'" << toName << "\' doesn't exist." << std::endl;
        std::cout << "Do you want to create a new config? y or n " << std::flush;
        std::cin >> t;
        if (t.length() > 1 || (t!="y" && t!="Y")){
            std::cout << "Stopping process" << std::endl;
            return 0;
        }
        
        data[toName] = {
            {"entry", ""},
            {"depends", json::array()},
            {"flags", json::array()},
            {"bin", ""},
            {"args", json::array()}
        };
    }
    i.close();

    std::ofstream o("play.json");
    if (o.fail()){
        PUT << "File creation for write operation failed" << std::endl;
        return 1;
    }
    o << std::setw(4) << data << std::endl;
    o.close();
    return 0;
}

int erase_name(std::string& toName){
    std::ifstream i("play.json");
    if (!i.is_open()){
        PUT << "Error while opening: play.json" << std::endl;
        if (i.fail()) PUT << "Badbit was set" << std::endl;
        if (i.fail()) PUT << "Error details: " << strerror(errno) << std::endl;
        return 1;
    }
    json data = json::parse(i);
    if (data.value("_active", "") == toName) data["_active"] = "";
    if (data.contains(toName)) data.erase(toName);
    i.close();


    std::ofstream o("play.json");
    if (o.fail()){
        PUT << "File creation for write operation failed" << std::endl;
        return 1;
    }
    o << std::setw(4) << data << std::endl;
    o.close();
    return 0;
}

int list(){
    std::ifstream i("play.json");
    if (!i.is_open()){
        PUT << "Error while opening: play.json" << std::endl;
        if (i.fail()) PUT << "Badbit was set" << std::endl;
        if (i.fail()) PUT << "Error details: " << strerror(errno) << std::endl;
        return 1;
    }
    json data = json::parse(i);

    for (auto& sp: data.items()){
        std::string key = sp.key();
        if (key != "_active" && key != "_lang" && key != "_compileToCwd") PUT << "  " << sp.key() << std::endl;
    }
    
    i.close();
    return 0;
}

int find_compiler(const std::string lang, const std::string entry, std::string* compiler){

    if(lang == "c"){
        *compiler = "gcc"; 
    } else if (lang == "c++"){
        *compiler = "g++";
    } else {
        // Invalid lang. Switch to entry point ext.
        // std::cout << "Language " << lang << " is not supported" << std::endl;
        size_t index_dot = entry.find_last_of('.');
        if(index_dot == std::string::npos){
            std::cout << "Sorry, neither is the lang config-ed nor the file has extension" << std::endl;
            return 1;
        }
        std::string ext = entry.substr(index_dot+1);
        *compiler = (ext == "c") ? "gcc" : ((ext == "cpp") ? "g++" : "null");
        if (*compiler == "null") {
            std::cout << "The file extension of entry-file is not supported" << std::endl;
            return 1;
        }
    }
    return 0;
}

int extractConfig(Config* config_){
    int err;
    
    std::ifstream configFile("play.json");
    json config = json::parse(configFile);
    bool binned = !config.value("_compileToCwd", true);

    if(!config.contains("_active")){
        PUT << "  Active field is a must" << std::endl;
        return 1;
    }
    std::string main = config.at("_active").get<std::string>();  

    if(!config.contains(main)) {
        PUT << "  The specified suite is absent" << std::endl;
        return 1;
    }
    std::map<std::string, json> suite = config.at(main).get<std::string, json>();

    if(suite.find("entry") == suite.end()) {
        PUT << "  The specified suite doesn't have entry" << std::endl;
        return 1;
    }
    std::string entry = suite.at("entry").get<std::string>();
    std::string bin;
    if(suite.find("bin") == suite.end()) {
        size_t index_dot = entry.find_last_of(".");
        bin = index_dot != std::string::npos? entry.substr(0, index_dot) + ".out" : entry+".out";
    } else bin = suite.at("bin").get<std::string>();

    std::string lang = config.value("_lang", "");
    std::string compiler;

    err = find_compiler(lang, entry, &compiler);

    if(err) return err;

    std::vector<std::string> depends ;
    std::vector<std::string> flags; 
    std::vector<std::string> args; 
    if(suite.find("depends") == suite.end()){
        depends = {};
    } else depends = suite.at("depends").get<std::vector<std::string>>();
    if(suite.find("flags") == suite.end()){
        flags = {};
    } else flags = suite.at("flags").get<std::vector<std::string>>();
    if(suite.find("args") == suite.end()){
        args = {};
    } else args = suite.at("args").get<std::vector<std::string>>();

    configFile.close();

    *config_ = {compiler, entry, bin, depends, flags, args, binned};

    return 0;
}

void convertConfig(Config* config, std::vector<char*>& args, std::vector<char *>& runtime_args, std::vector<std::string>& storage){
    storage.reserve(4+config->flags.size()+config->dependencies.size()); // g** entry -o bin -flags -depends nullptr 
    storage.push_back(config->compiler);
    storage.push_back(config->entry);
    storage.push_back("-o");
    storage.push_back(config->binned? "./bin/"+config->bin :"./"+config->bin);
    for(std::string sp: config->dependencies){
        storage.push_back(sp);
    }
    for(std::string sp: config->flags){
        storage.push_back("-"+sp);
    }
    
    args.reserve(storage.size() + 1);
    for(std::string& val: storage){
        args.push_back(val.data());
    } 

    args.push_back(nullptr);
    
    runtime_args.reserve(1 + config->args.size() + 1);
    runtime_args.push_back(args.at(3));
    for(std::string& val: config->args)
        runtime_args.push_back(val.data());
    runtime_args.push_back(nullptr);
}

int runFromConfig(std::string& bin, bool& compFail){
    Config config;
    int err = extractConfig(&config);  

    std::vector<char *> args;
    std::vector<std::string> tmp;
    std::vector<char *> runtime_args;
    convertConfig(&config, args,runtime_args, tmp); 

    if(config.binned) std::filesystem::create_directories("./bin/");
    err = runProcess(config.compiler.data(), args);

    if (err){
        PUT << "Something went wrong during compilation" << std::endl;
        compFail = true;
        return 5;
    }

    err = runProcess(args.at(3), runtime_args);
    bin = std::string(args.at(3));
    tmp.clear();
    return err;
}