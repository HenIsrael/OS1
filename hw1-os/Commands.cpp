#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

extern SmallShell &smash;
const std::string WHITESPACE = " \n\r\t\f\v";


#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}
//-----------------------Helper Functions-------------------------------------------------------------------------
/*

static vector<string> command_line_decoder(const char* cmd_line){
  string cmd_s = _trim(string(cmd_line));
	vector<string> command;
  while (!cmd_s.empty()) {
		string word = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
		if(word != "") {
			command.push_back(word);
		}
		cmd_s.erase(0, word.length() + 1);;
	}
  return command;

}
*/
void freeArgs(char ** args, int len){
  if(!args)
    return;
  for (int i = 0; i < len; ++i)
  {
    if(!args[i])
        continue;
    free(args[i]);
    args[i] = NULL;
  }
  free(args);
}

bool isItNumber(const string &str){
  if (str.empty() || ((!isdigit(str[0])) && (str[0] != '-') && (str[0] != '+'))){
    return false;
  } 

  for(unsigned int i=1; i<str.length(); i++){
      if(isdigit(str[i]) == 0){
          return false;
      }
  }

  return true;
}


// TODO: Add your implementation for classes in Commands.h 
//-----------------------Classes code implementation----------------------------------------------------------------
Command::Command(const char* cmd_line){
  int len_cmd_line = strlen(cmd_line);
  char* new_command_line = new char[len_cmd_line + 1];
  strcpy(new_command_line, cmd_line);
  this->command_line = new_command_line;

  char ** args = (char**)malloc(sizeof(char*) * COMMAND_MAX_ARGS);
  int num_of_args = _parseCommandLine(cmd_line, args);

  for(int i = 1; i < num_of_args; i++) { 
    
    this->params.push_back(string(args[i]));
  }
  
  freeArgs(args, COMMAND_MAX_ARGS);
  
}

Command::~Command(){
  delete this->command_line;
}
const char* Command::getCommandLine() const{
  return this->command_line;
}
bool Command::isStopped() const{
  return this->stopped;
}
void Command::setStopped(bool stopped){
  this->stopped = stopped;
}
bool Command::isBackground() const {
    return this->background;
}
void Command::setBackground(bool background) {
    this->background = background;
}
bool Command::isExternal() const {
    return this->external;
}



BuiltInCommand::BuiltInCommand(const char* cmd_line) : Command(cmd_line){}

// TODO: [1] - class ExternalCommand
//       [2] - class PipeCommand
//       [3] - class RedirectionCommand



JobsList::JobEntry::JobEntry(int jobId, int pid, Command* cmd) : command(cmd){
  this->job_id = jobId;
  this->pid = pid;
  this->start = time(nullptr);
  if(this->start == ERROR){
    perror("smash error: time failed");
  }
}

int JobsList::JobEntry::getJobId() const{
  return this->job_id;
}
void JobsList::JobEntry::setJobId(int JobId){
  this->job_id = JobId;
}
pid_t JobsList::JobEntry::getPid() const{
  return this->pid;
}
void JobsList::JobEntry::setPid(int pid){
  this->pid = pid;
}
time_t JobsList::JobEntry::getTimeCommand() const{
  return this->start;
}
void JobsList::JobEntry::setTimeCommand(time_t time){
  this->start = time;
}
const char* JobsList::JobEntry::getCommand() const{
  return this->command->getCommandLine();
}
void JobsList::JobEntry::deleteCommand(){
  delete this->command;
}
bool JobsList::JobEntry::isStopped() const{
  return this->command->isStopped();                  
}
void JobsList::JobEntry::setStopped(bool stopped){
  return this->command->setStopped(stopped);          
}
bool JobsList::JobEntry::isBackground() const{
  return command->isBackground();                    
}
void JobsList::JobEntry::setBackground(bool background){
  return this->command->setBackground(background);          
}

// TODO : add class JobsList code here ->

int JobsList::addJob(int pid, Command *cmd, bool stopped) {
    int new_job_id = getMaxFromJobs();                                           
    new_job_id += 1;
    JobEntry new_job(new_job_id, pid, cmd);

    this->run_jobs.insert(std::pair<int, JobEntry>(new_job_id, new_job));
    setMaxFromJobs(new_job_id);                                                  

    return new_job_id;
}

int JobsList::getMaxFromJobs() const{
  return this->max_from_jobs;
}

void JobsList::setMaxFromJobs(int max_job_id){
  this->max_from_jobs = max_job_id;
}

int JobsList::getMaxFromStoppedJobs() const {
    return this->max_from_stopped;
}

void JobsList::setMaxFromStoppedJobs(int max_stopped_job_id) {
    this->max_from_stopped = max_stopped_job_id;
}

int JobsList::getJobIdByPid(int pid){
  if(this->run_jobs.size() == 0){
    return 0;
  }
  for(const auto job : this->run_jobs){
    if(job.second.getPid() == pid){
      return job.first;
    }
  }
  return 0;
}

int JobsList::MaxJobInMap(){
  if (this->run_jobs.size() == 0) {
      return 0;
  }
  int max_job_id = 0;
  for (const auto &job : this->run_jobs) {
      if (job.first > max_job_id) {
        max_job_id = job.first;
      }
  }

  return max_job_id;  
}

void JobsList::removeJobById(int jobId){
  JobEntry job = this->run_jobs.find(jobId)->second;
  job.deleteCommand();

  this->run_jobs.erase(jobId);
  int max_job_id = MaxJobInMap();

  setMaxFromJobs(max_job_id);
}

void JobsList::removeFinishedJobs(){
  int status;
  int child_pid = waitpid(-1, &status, WNOHANG);
  while (child_pid > 0){

    int job_id = getJobIdByPid(child_pid);
    if (job_id != 0){
      removeJobById(job_id);
    }

    int child_pid = waitpid(-1, &status, WNOHANG);
  }
}

void JobsList::printJobsList(){
  for(auto &job : this->run_jobs){
    time_t now = time(nullptr);
    if(now == ERROR){
      perror("smash error: time failed");
      return;
    }

    time_t job_start_time = job.second.getTimeCommand();
    time_t seconds_elapsed = difftime(now, job_start_time);

    cout << "[" << job.second.getJobId() << "] " << job.second.getCommand() << " : " << job.second.getPid() << " " << seconds_elapsed << " secs";
    if(job.second.isStopped()){
      cout << " (stopped)";
    }

    cout << endl;
  }
}

bool JobsList::isJobExistsById (int job_id)
{
  std::map<int,JobEntry>::iterator it;
  it = this->run_jobs.find(job_id);
  if (it == this->run_jobs.end())
  {
    return false;
  }
  else
  {
    return true;
  }
}

const map<int, JobsList::JobEntry> &JobsList::getRunJobs() const{
  return this->run_jobs;
}

void JobsList::ChangeLastStoppedJob() {

    map<int, JobsList::JobEntry> run_jobs = smash.getJobsList()->getRunJobs();
    if (run_jobs.size() == 0) {
        this->max_from_stopped = 0;
    }
    int max = 0;
    for (auto job : run_jobs){
        if(job.first > max && job.second.isStopped()) {
            max = job.first;
        }
    }
    this->max_from_stopped  = max;
}


SmallShell::SmallShell() : jobs(JobsList()) {
// TODO: add your implementation
// TODO- create malloc to adress that will point to null for lastPwd
// TODO - add constructor to jobs
lastPwd=new char*;
*lastPwd=nullptr;
//lastPwd=new char*;
}


SmallShell::~SmallShell() {
// TODO: add your implementation
   delete[] lastPwd; 
}


/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:
/*
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("chprompt") == 0) {
    return new ChpromptCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0){
    return new ChangeDirCommand(cmd_line, this->lastPwd);
  }
 
  else if (firstWord.compare("jobs") == 0) {
    return new JobsCommand(cmd_line, smash.getJobsList());
  }
  else if (firstWord.compare("fg") == 0) {
    return new ForegroundCommand(cmd_line, smash.getJobsList());
  }
  else if (firstWord.compare("quit") == 0) {
    return new QuitCommand(cmd_line, smash.getJobsList());
  }
  else if (firstWord.compare("bg") == 0) {
    return new BackgroundCommand(cmd_line, smash.getJobsList());
  }
  else if (firstWord.compare("kill") == 0) {
    return new KillCommand(cmd_line, smash.getJobsList());
  }
  
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
  Command* cmd = CreateCommand(cmd_line);
  if(cmd){
    cmd->execute();
  }
}

string SmallShell::getPrompt(){
  return this->prompt;
}

void SmallShell::setPrompt(string newprompt){
  this->prompt = newprompt;
}

JobsList* SmallShell::getJobsList(){
  return &(this->jobs);
}

char** SmallShell::getLastPwd(){
  return this->lastPwd;
}


void SmallShell::setLastPwd(char *newPwd){
  (*this->lastPwd) = newPwd;
}

//-----------------------Built in commands------------------------------------------------------------------------
ChpromptCommand::ChpromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}
void ChpromptCommand::execute(){
  if (this->params.empty()){
    smash.setPrompt("smash> ");
  }
  else{
        string newPromptName = this->params.at(0);
        newPromptName.append("> ");
        
        smash.setPrompt(newPromptName);
  }   
}

ShowPidCommand::ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){}

void ShowPidCommand::execute()
{
  std::cout << "smash pid is "<< ::getpid() << endl;
}

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}
void GetCurrDirCommand::execute(){
  char* path = getcwd(NULL,0);
  if(!path){
    perror("smash error: getcwd failed");
  }
  else{
    cout << path <<endl;
  }

  free(path);
}


ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char** plastPwd): BuiltInCommand(cmd_line)
{
  //std::cout << "ok- plastPwd-next path in consructoe is "  << endl ;
  //std::cout << "plastPwd is " << *plastPwd << endl ;
  if(this->params.size() == 0)
  {
    this->status = no_args;
    return;
  }
  char* current_path = getcwd(NULL,0);
  std::string path = this->params.at(0);
  m_next_plastPwd = current_path;
  if(this->params.size() > 1)
  {
    this->status = too_many_arg;
  }
  else if(path == "-")
  {

    if(!*plastPwd)
    {
      status = back_null;
    }
    else
    {
      status = back ;
      std::cout << "im here1 "  << endl ;
      std::cout << "im here addres "  << plastPwd<< endl ;
      std::cout << "im here value "  << *plastPwd<< endl ;

      next_path = *plastPwd ;
      std::cout << "back- plastPwd-next path in consructoe is " << next_path << endl ; 
    }
  } 
  else
  {
    this->status = ok_cd ; 
    std::cout << "im here2 "  << endl ;
    next_path = path ; 
    std::cout << "ok- plastPwd-next path in consructoe is " << next_path << endl ;
  }
}


void ChangeDirCommand::execute()
{
  if(this->status == no_args)
  {
    return;
  }
  else if(this->status == too_many_arg)
  {
    std::cerr << "smash error: cd: too many arguments" <<endl ;
    return;
  }
  else if(this->status == back_null)
  {
    std::cerr << "smash error: cd: OLDPWD not set" <<endl ;
    return;
  } 
  else
  {
    if(chdir(next_path.c_str()) == ERROR)
    {
      perror("chdir failed");
    }
    else
    {
      char* tmp = strdup(m_next_plastPwd.c_str());

      smash.setLastPwd( tmp );

      std::cout << "lastPwd shell is" << *(smash.getLastPwd()) <<endl ;
      //delete tmp; // check if needed
    }
  }
}


ForegroundCommand::ForegroundCommand(const char* cmd_line, JobsList* jobs)
:BuiltInCommand(cmd_line),jobs_list(jobs)
{
  if(this->params.size() == 0)
  {
    int max_job = this->jobs_list->MaxJobInMap();
    if(max_job == 0)
    {
      this->status = no_jobs;
      return;
    }
    else
    {
      this->status = ok;
    }
  }
  else if(this->params.size() > 1)
  {
    this->status = invalid_arguments;
    return;
  }
  else if(!isItNumber(this->params.at(0)))
  {
    this->status = invalid_arguments;
    return;
  }
  else
  {
    if(this->jobs_list->isJobExistsById(stoi(this->params.at(0))))
    {
      this->status = ok;
    }
    else
    {
      this->status = job_not_exist;
    }
  }
}

void ForegroundCommand::execute()
{
  if( this->status == no_jobs)
  {
    std::cerr << "smash error: fg: jobs list is empty" << endl;
  }
  else if( this->status == job_not_exist )
  {
    std::cerr << "smash error: fg: job-id " << this->params.at(0) << " does not exist" << endl;
  }
  else if( this->status == invalid_arguments )
  {
    std::cerr << "smash error: fg: invalid arguments" << endl ;
  }
  else
  {
    this->jobs_list->removeFinishedJobs();

    std::map<int,JobsList::JobEntry> run_jobs = this->jobs_list->getRunJobs(); //here
    int job_id = stoi(this->params.at(0));
    std::string command_to_print = jobs_list->getRunJobs().find(job_id)->second.getCommand();
    int job_pid = jobs_list->getRunJobs().find(job_id)->second.getPid();

    std::cout <<  command_to_print << " : " << job_pid <<endl; 

    run_jobs.find(job_id)->second.setBackground(false);
    //this->jobs_list->getRunJobs().find(job_id)->second.setBackground(false); // here

    if( kill(job_pid, SIGCONT) == ERROR )
    {
      perror("smash error: kill failed");
      return;
    }

    run_jobs.find(job_id)->second.setStopped(false);

    waitpid(job_pid , NULL , NULL); //check if correct
    this->jobs_list->removeJobById(job_id);
    this->jobs_list->ChangeLastStoppedJob();

}
}


JobsCommand::JobsCommand(const char* cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs_list(jobs) {}

void JobsCommand::execute()
{
  JobsList* list = smash.getJobsList();
  list->removeFinishedJobs();
  list->printJobsList();
}

QuitCommand::QuitCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line), jobs_list(jobs)
{
  if (this->params.size() != 0 && this->params.at(0)== "kill")
  {
    this->kill_bool = true ; 
  }
}

  void QuitCommand::execute()
  {
    this->jobs_list->removeFinishedJobs(); //for safety
    if(this->kill_bool)
    {
      //std::cout << "im here bitches" <<endl;
      int map_size = this->jobs_list->getRunJobs().size();
      std::cout << "sending SIGKILL signal to " << map_size <<" jobs" <<endl; 
      map<int, JobsList::JobEntry>::iterator it;
      map<int, JobsList::JobEntry> run_jobs;

      for (it = run_jobs.begin(); it != run_jobs.end(); it++)
      {
        int job_id = it->second.getJobId();
        std::cout << it->second.getPid() << ": " << it->second.getCommand() <<endl;
        kill( it->second.getPid() ,SIGKILL );
        this->jobs_list->removeJobById(job_id);
      }
    }
    //kill(getpid() ,SIGKILL );
    exit(0);
  }



/*
ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line),jobs_list(jobs) {} 
void ForegroundCommand::execute(){
  if(this->params.size() >1){
    cerr << "smash error: fg: invalid arguments" << endl;
    return;
  }

  map<int, JobsList::JobEntry> run_jobs = this->jobs_list->getRunJobs();
  int job_id;

  if(this->params.empty()){
    if(run_jobs.size() == 0){
      cerr << "smash error: fg: jobs list is empty" << endl;
      return;
    }
    //TODO: maybe need to remove finished jobs before?
    job_id = this->jobs_list->MaxJobInMap();
  }
  else {
    if(!isItNumber(this->params.at(0))){
    
      cerr << "smash error: fg: invalid arguments" << endl;
      return; 
    }

    int job_required = stoi(this->params.at(0));
    this->jobs_list->removeFinishedJobs();

    if(run_jobs.find(job_required) == run_jobs.end()){
      cerr << "smash error: fg: job-id " << job_required << " does not exist" << endl;
      return;
    }

    job_id = job_required;
  }

  int job_pid = run_jobs.find(job_id)->second.getPid();
  string command_line = run_jobs.find(job_id)->second.getCommand();

  cout << command_line << " : "  << job_pid << endl;

  run_jobs.find(job_id)->second.setBackground(false);
  if(killpg(job_pid, SIGCONT) == ERROR){
    perror("smash error: kill failed");
    return;
  }

  run_jobs.find(job_id)->second.setStopped(false);
  waitpid(job_pid, nullptr, WUNTRACED);

  if(!run_jobs.find(job_id)->second.isStopped()) {
      this->jobs_list->removeJobById(job_id);
  }

  this->jobs_list->ChangeLastStoppedJob();
}
*/

BackgroundCommand::BackgroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line),jobs_list(jobs) {}
void BackgroundCommand::execute() {
  if(this->params.size() >1){
    cerr << "smash error: bg: invalid arguments" << endl;
    return;
  }
  map<int, JobsList::JobEntry> run_jobs=this->jobs_list->getRunJobs();
  int job_id = 0;

  if(this->params.size() == 0){
    job_id = this->jobs_list->MaxJobInMap();

    if(job_id == 0){
      cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
      return;
    }
  }

  else{
    if(!isItNumber(this->params.at(0))){
      cerr << "smash error: bg: invalid arguments" << endl;
      return; 
    }
    else {
      job_id = stoi(this->params.at(0));
    }

    if(run_jobs.find(job_id) == run_jobs.end()){
      cerr << "smash error: bg: job-id " << job_id << " does not exist" << endl;
      return;
    }

    if(run_jobs.find(job_id)->second.isBackground() && !run_jobs.find(job_id)->second.isStopped()){
      cerr << "smash error: bg: job-id " << job_id << " is already running in the background" << endl;
      return;
    }
    else if(run_jobs.find(job_id)->second.isBackground() && run_jobs.find(job_id)->second.isStopped()){
      int job_pid = run_jobs.find(job_id)->second.getPid();

      string command_line = run_jobs.find(job_id)->second.getCommand();
      cout << command_line << " : "  << job_pid << endl;

      if(killpg(job_pid, SIGCONT) == ERROR){
        perror("smash error: kill failed");
        return;
      }

      run_jobs.at(job_id).setStopped(false);
      this->jobs_list->ChangeLastStoppedJob();
      return;
    }
  }

  int job_pid = run_jobs.find(job_id)->second.getPid();

  string command_line = run_jobs.find(job_id)->second.getCommand();
  cout << command_line << " : "  << job_pid << endl;

  if(killpg(job_pid, SIGCONT) == ERROR){
    perror("smash error: kill failed");
    return;
  }

  run_jobs.at(job_id).setStopped(false);
  this->jobs_list->ChangeLastStoppedJob();
}

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line) ,jobs_list(jobs){}
void KillCommand::execute() {

  if (this->params.size() != 2) {
      cerr << "smash error: kill: invalid arguments" << endl;
      return;
  }

  int sig_num = 0;
  int job_id = 0;

  if (!isItNumber(this->params.at(0)) || !isItNumber(this->params.at(1))) { //TODO: 
    cerr << "smash error: kill: invalid arguments" << endl;
    return;
  }
  else {
    sig_num = stoi(this->params.at(0));
    job_id = stoi(this->params.at(1));
  }

  if(job_id < 0){
    cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
    return;
  }
  if(sig_num >= 0){
     cerr << "smash error: kill: invalid arguments" << endl;
     return;
  }

  map<int, JobsList::JobEntry> run_jobs=this->jobs_list->getRunJobs();
  if (run_jobs.find(job_id) == run_jobs.end()){
      cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
      return;
  }

  int job_pid = run_jobs.find(job_id)->second.getPid();
  if(kill(job_pid, abs(sig_num)) == ERROR){
    perror("smash error: kill failed");
    return;
  }
  else if(abs(sig_num) == 9){
    this->jobs_list->removeJobById(job_id);
  }

  else if(abs(sig_num) == 19){
    //TODO : check if is valid command
  }
  cout << "signal number " << abs(sig_num) << " was sent to pid " << job_pid << endl;
}
