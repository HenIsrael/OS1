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

//-----------------------Built in commands------------------------------------------------------------------------
ChpromptCommand::ChpromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}
void ChpromptCommand::execute(){
  if (this->params.empty()){
    smash.SetPrompt("smash> ");
  }
  else{
        string newPromptName = this->params.at(0);
        newPromptName.append("> ");
        smash.SetPrompt(newPromptName);
  }

   
}
ShowPidCommand::ShowPidCommand(const char* cmd_line)
{
// TODO add constructor here
}

void ShowPidCommand::execute()
{
  std::cout << "smash pid is "+ getpid();
}

// TODO: Add your implementation for classes in Commands.h 
//-----------------------Classes code implementation----------------------------------------------------------------

Command::Command(const char* cmd_line){
  int len_cmd_line = strlen(cmd_line);
  char* new_command_line = new char[len_cmd_line + 1];
  strcpy(new_command_line, cmd_line);
  this->command_line = new_command_line;

  char ** args = (char**)malloc(sizeof(char*) * MAX_ARGS);
  int num_of_args = _parseCommandLine(cmd_line, args);

  
  for(unsigned int i = 1; i <= num_of_args; i++) {
    this->params.push_back(args[i]);
  }

  freeArgs(args, MAX_ARGS);
}

Command::~Command(){delete this->command_line;}  

  
SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
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
    return new ChmodCommand(cmd_line);
  }
   
  
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

string SmallShell::GetPrompt(){
  return this->prompt;
}

void SmallShell::SetPrompt(string newprompt){
  this->prompt = newprompt;
}

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
