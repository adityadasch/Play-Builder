#include "common.hpp"
int runProcess(const char* filename, const std::vector<char *> args){
    pid_t pid = fork();

    if(pid == -1){
        // Error
        return 1;
    } else if(pid == 0){
        // Child process
        execvp(filename, args.data());
        _exit(127);
    }
    // Parent
    int status;
    if(waitpid(pid, &status, 0) == -1) return 1;
    if(WIFEXITED(status)){
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)){
        return WTERMSIG(status); // error
    }

    return 1;
}