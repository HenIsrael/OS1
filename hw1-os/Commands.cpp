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

// TODO: Add your implementation for classes in Commands.h 
//-----------------------Classes code implementation----------------------------------------------------------------
Command::Command(const char* cmd_line){
  int len_cmd_line = strlen(cmd_line);
  char* new_command_line = new char[len_cmd_line + 1];
  strcpy(new_command_line, cmd_line);
  this->command_line = new_command_line;

  char ** args = (char**)malloc(sizeof(char*) * COMMAND_MAX_ARGS);
  int num_of_args = _parseCommandLine(cmd_line, args);

  for(unsigned int i = 1; i < num_of_args; i++) { 
    
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
int JobsList::JobEntry::getPid() const{
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


SmallShell::SmallShell() {
// TODO: add your implementation
// TODO- create malloc to adress that will point to null for lastPwd
lastPwd=new char*;
*lastPwd=nullptr;
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
  /*
  else if (firstWord.compare("cd") == 0){
    return new ChangeDirCommand(cmd_line, this->lastPwd);
  }
  */
  else if (firstWord.compare("jobs") == 0) {
    return new JobsCommand(cmd_line, smash.getJobsList());
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
  std::cout << "smash pid is "+ getpid() << endl;
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

/*
ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char** plastPwd): BuiltInCommand(cmd_line)
{
  //errors

  string path = this->params.at(0);

  if(this->params.size() > 1)
  {
    std::cout << "smash error: cd: too many arguments";
  }
  else if(path == "-" && !*plastPwd)
  {
    std::cout << "smash error: cd: OLDPWD not set";
  }
  
  
  else
  {

    getcwd(current_path.c_str() ,COMMAND_ARGS_MAX_LENGTH);
    if(path == "-")
    {
      next_path = *plastPwd;
    }
    else
    {
      next_path = path;
    }
    *plastPwd = current_path;
  }

}


ChangeDirCommand::~ChangeDirCommand()
{
  delete current_path;
  delete next_path; 
}


void ChangeDirCommand::execute()
{
  if(chdir(next_path.c_str()) == ERROR)
    {
      perror("OLDPWD not set");
    }
}


*/

JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs_list(jobs) {}
void JobsCommand::execute(){
  JobsList* list = smash.getJobsList();
  list->removeFinishedJobs();
  list->printJobsList();
}

