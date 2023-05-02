#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;
extern SmallShell &smash;

void ctrlZHandler(int sig_num) {

	// TODO: Add your implementation
  signal(SIGSTOP, &ctrlZHandler);
  std::cout << "smash: got ctrl-Z" << std::endl;
  pid_t fgprocess = smash.getFgProcess();
  if (fgprocess != 0) {
    int jobID = smash.getJobsList()->getJobIdByPid(fgprocess);
    map<int, JobsList::JobEntry> run_jobs=smash.getJobsList()->getRunJobs();
    run_jobs.find(jobID)->second.setStopped(true);
    
        
      if (kill(fgprocess, SIGSTOP) == ERROR) {
        perror("smash error: kill failed");
        return;
      }
        std::cout << "smash: process " << fgprocess << " was stopped" << std::endl;
    }
}


void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

