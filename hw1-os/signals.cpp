#include <iostream>
#include <signal.h>
#include "signals.h"
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

void alarmHandler(int sig_num) 
{
  printf("here -2");
  // TODO: Add your implementation
  cout << "smash: got an alarm" << endl ;
  
  printf("here -1");
  if(smash.getTimeList()->getTimeMap().size() == 0)
  {
    return;
  }
  printf("here 0");
  //int max_job_id = 0;
  for (const auto &id : smash.getTimeList()->getTimeMap()) 
  {
    printf("here 1");
    int job_id = smash.getTimeList()->Get_JobId_Of_Finished_Timeout(time(nullptr));
    int TimeId =smash.getTimeList()->Get_TimeId_Of_Finished_Timeout(time(nullptr));
    printf("here 2");
      if (TimeId != -1) 
      {
        if( kill(smash.getTimeList()->getTimeMap().find(TimeId)->second.getPid() , SIGKILL ) == ERROR)
        {
          perror("smash error: kill failed");
          return;
        } 
        else
        {
          printf("here 3");
          smash.getTimeList()->removeTimeById(TimeId);
          if (smash.getFgProcess() != 0)
          {
            smash.getJobsList()->removeJobById(smash.getTimeList()->Get_JobId_Of_Finished_Timeout(time(nullptr)));
          }
          
          smash.getTimeList()->changeMaxTimeId();
          smash.getJobsList()->ChangeLastStoppedJob();
          smash.getJobsList()->removeFinishedJobs();
          //update last of allllll things

        }
      }
  }
}

