#include <iostream>
#include <signal.h>
#include "signals.h"
#include <sys/wait.h>
#include "Commands.h"

using namespace std;
extern SmallShell &smash;

void ctrlZHandler(int sig_num) {

  cout << "smash: got ctrl-Z" << std::endl;
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
  cout << "smash: got ctrl-C" << endl ;
 
  if((smash.getFgProcess()) != 0 )
  {
    if( kill(smash.getFgProcess() , SIGKILL ) == ERROR)
    {
      perror("smash error: kill failed");
      return;
    }
    cout << "smash: process " << smash.getFgProcess() << " was killed" << endl ;
  }
}

void alarmHandler(int sig_num) {
  
  // TODO: Add your implementation
  std::cout << "smash: got an alarm" << std::endl;
  time_t now = time(nullptr);
  int time_id = smash.getTimeList()->Get_TimeId_Of_Finished_Timeout(now);

  if (time_id != -1){
    // case there is a process get timeout
    int pid = smash.getTimeList()->getTimeMap().find(time_id)->second.getPid();
    int job_id = smash.getTimeList()->Get_JobId_Of_Finished_Timeout(now);
    int status;
    if(waitpid(pid, &status, WNOHANG) != 0){
      // if process terminated
      smash.getJobsList()->removeJobById(job_id);
      smash.getTimeList()->removeTimeById(time_id);
      smash.getTimeList()->changeMaxTimeId();
    }else{
      if(killpg(pid, SIGINT) == ERROR){
        perror("smash error: kill failed");
        return;
      }

      char* command_timeout = smash.getTimeList()->getTimeMap().find(time_id)->second.getCommand();
      std::cout << "smash: " << command_timeout << " timed out!" << std::endl;
      if (smash.getJobsList()->getRunJobs().find(job_id)->second.isBackground() || smash.getJobsList()->getRunJobs().find(job_id)->second.isStopped()){
        // if the process running in the back or stopped we need to remove it from the jobs list
        smash.getJobsList()->removeJobById(job_id);
      }

      smash.getTimeList()->removeTimeById(time_id);
      smash.getTimeList()->changeMaxTimeId();
    }
  }

  if (smash.getTimeList()->getMaxId() != 0){
    smash.getTimeList()->What_is_the_Next_Timeout(now);
  }
}

