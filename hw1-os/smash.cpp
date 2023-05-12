#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

SmallShell& smash = SmallShell::getInstance();

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }


    struct sigaction sig ;
    sig.sa_handler = &alarmHandler;
    sig.sa_flags = SA_RESTART;


    if(sigaction(SIGALRM , &sig , NULL) != 0) {
        perror("smash error: failed to set alarm handler");
    }

    //TODO: setup sig alarm handler
    

    while(true) {  
        std::cout << smash.getPrompt();
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.getJobsList()->removeFinishedJobs();
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}