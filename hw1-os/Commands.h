#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <map>
#include <time.h>

using std::vector;
using std::string;

using namespace std;

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define ERROR (-1)

enum status_cd {too_many_arg , no_args, back , back_null , ok_cd };
enum error_status_fg {no_jobs , job_not_exist , invalid_arguments , ok };

class Command {
// TODO: Add your data members
 protected:
  const char* command_line ;
  vector<string> params; 
  bool external = false;
  bool stopped = false;
  bool background = false;
  bool foreground= false;

 
 public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
  const char* getCommandLine() const;
  bool isStopped() const;
  void setStopped(bool stopped);
  bool isBackground() const;
  void setBackground(bool background);
  bool isExternal() const;
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line, bool is_back);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};


//-------------------------------------Built in simple commands-----------------------------------------------

class ChpromptCommand : public BuiltInCommand{
  public:
    explicit ChpromptCommand(const char* cmd_line);
    virtual ~ChpromptCommand(){}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
private:
  status_cd status;
  //std::string current_path = "";
  std::string next_path = "";
  std::string m_next_plastPwd = "";
public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;

// TODO : TALI add getters setters !:) 
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class TimeList;
class QuitCommand : public BuiltInCommand {
private:
  bool kill_bool = false ;
  JobsList* jobs_list ;

public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};


class JobsList {
 public:
  class JobEntry {
   // TODO: Add your data members
  private:
    int job_id;
    int pid;
    time_t start;
    Command* command;
    int duration = 0; //new

  public:
    JobEntry(int jobId, int pid, Command* cmd);
    ~JobEntry(){};
    int getJobId() const;
    void setJobId(int JobId);
    pid_t getPid() const;
    void setPid(int pid);
    time_t getTimeCommand() const;
    void setTimeCommand(time_t time);
    const char* getCommand() const;
    void deleteCommand();
    bool isStopped() const;
    void setStopped(bool stopped);
    bool isBackground() const;
    void setBackground(bool background);
  };

 private:
  // TODO: Add your data members
  map<int, JobEntry> run_jobs;
  int max_from_jobs = 0;
  int max_from_stopped = 0;

 public:
  JobsList()=default;
  ~JobsList(){};
  //void addJob(Command* cmd, bool isStopped = false);
  void printJobsList();
  //void killAllJobs();
  void removeFinishedJobs();
  //JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  //JobEntry * getLastJob(int* lastJobId);
  //JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
  int addJob(int pid, Command* cmd, bool stopped);
  int getMaxFromJobs() const;
  void setMaxFromJobs(int max_job_id);
  int getMaxFromStoppedJobs() const;
  void setMaxFromStoppedJobs(int max_stopped_job_id);
  int MaxJobInMap();
  int getJobIdByPid(int pid);
  void ChangeLastStoppedJob();
  const std::map<int, JobEntry> &getRunJobs() const;
  bool isJobExistsById (int job_id);

  /*
TODO : [*] - ???void addJob(Command* cmd, bool isStopped = false);???
       [*] - void killAllJobs();
       [*] - JobEntry * getJobById(int jobId);
       [*] - JobEntry * getLastJob(int* lastJobId);
       [*] - JobEntry *getLastStoppedJob(int *jobId);
*/
};

class TimeList{
  public:
    class TimeEntry{
    private:
      int id = 0;
      int job_id = 0;
      int pid = 0;
      int time_of_dur = 0;
      char *command;
      time_t time_of_command_came;
    public:
      TimeEntry(int id, int jobID, int pid, int timeOfDur, char *command);
      int getJobId() const;
      int getPid() const;
      int getTimeOfDur() const;
      char* getCommand() const;
      time_t getTimeOfCommandCame() const;
      ~TimeEntry() {};
    };

  private:
    int maxTimeId = 0;
    map<int, TimeEntry> timeMap;
  
  public:
    TimeList()=default;
    int getMaxId() const;
    int getMaxKeyInMap() const;
    void setMaxTimeId(int maxTimeEnteryId);
    int addTime(int job_id, int pid, int timeOfDur, char *command);
    void removeTimeById(int time_entery_Id);
    int Get_TimeId_Of_Finished_Timeout(time_t now);
    int Get_JobId_Of_Finished_Timeout(time_t now);
    void changeMaxTimeId();
    void What_is_the_Next_Timeout(time_t now);
    const std::map<int, TimeEntry> &getTimeMap() const;
    ~TimeList() {};
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 private:
  JobsList* jobs_list;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 private:
 JobsList *jobs_list;
 error_status_fg status ;
 
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 private:
  JobsList* jobs_list;
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TimeoutCommand : public Command {
/* Bonus */
// TODO: Add your data members
 public:
  explicit TimeoutCommand(const char* cmd_line, bool is_back);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  ChmodCommand(const char* cmd_line);
  virtual ~ChmodCommand() {}
  void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  GetFileTypeCommand(const char* cmd_line);
  virtual ~GetFileTypeCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  SetcoreCommand(const char* cmd_line);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 private:
  JobsList *jobs_list;
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class SmallShell {
 private:
  // TODO: Add your data members
  SmallShell();
  JobsList jobs;
  TimeList times;
  string prompt = "smash> ";
  char** lastPwd;
  int fg_process;
 public:
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
  string getPrompt();
  void setPrompt(string prompt);
  JobsList* getJobsList();
  TimeList* getTimeList();
  //  TODO: TALI  add getters setters here!:)
  char** getLastPwd();
  void setLastPwd(char *lastPwd);
  void setFgProcess(pid_t process_fg);
  pid_t getFgProcess() const;

};

#endif //SMASH_COMMAND_H_

#ifndef EXTRA
#define EXTRA
void freeArgs(char ** args, int len);
bool isItNumber(const string &str);

#endif // EXTRA