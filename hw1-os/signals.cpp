#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;
extern SmallShell &smash;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
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
    cout << " smash: process " << smash.getFgProcess() << " was killed" << endl ;
  }
}

void alarmHandler(int sig_num) {
  
  // TODO: Add your implementation
}

