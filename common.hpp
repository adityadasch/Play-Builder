#pragma once
#include <vector>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

int runProcess(const char* filename, const std::vector<char *> args);

struct Config{
    std::string compiler;
    std::string entry;
    std::string bin;
    std::vector<std::string> dependencies;
    std::vector<std::string> flags;
    std::vector<std::string> args;
    bool binned;
};