#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "Commands.h"
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <csignal>
#include <fcntl.h>

using namespace std;
const std::string WHITESPACE = " \n\r\t\f\v";

class file_status;

class file_status;

class file_status;

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

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() :shell_prompt("smash"),prev_dir(""),jobs(new JobsList),running_id(-1),running_pid(-1),running_cmd("")
{
    shell_pid = getpid();
    if(shell_pid==-1)
    {
        perror("smash error: getpid failed");
        return;
    }

}

SmallShell::~SmallShell() {
    delete jobs;

}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if (cmd_s == "")
    {
        return nullptr;
    }
    if (cmd_s.find('>') != string::npos)
    {
        return new RedirectionCommand(cmd_line);
    }
    if (cmd_s.find('|') != string::npos) {
        return new PipeCommand(cmd_line);
    }
    if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    else if (firstWord.compare("chprompt") == 0) {
        return new ChpromptCommand(cmd_line);
    }
    else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
    }
    else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line,jobs);
    }
    else if (firstWord.compare("fg") == 0) {
        return new ForegroundCommand(cmd_line,jobs);
    }
    else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line,jobs);
    }
    else if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line,jobs);
    }
    else if(firstWord.compare("chmod")==0) {
        return new ChmodCommand(cmd_line);
    }
    else {
        return new ExternalCommand(cmd_line);
    }
}
void SmallShell::executeCommand(const char *cmd_line)
{
    jobs->removeFinishedJobs();
    Command* cmd = CreateCommand(cmd_line);
    if(cmd!= nullptr) {
        cmd->execute();
    }
}

void JobsList::addJob(Command *cmd,int pid,Status status) {
    removeFinishedJobs();
    int id=0;
    if(jobsList.empty())
    {
        id=1;
    }else
    {
        id=jobsList[jobsList.size()-1]->job_id+1;

    }
    shared_ptr<JobEntry> newjob= make_shared<JobEntry>(JobEntry(pid,id,status,cmd->getCMD()));
    jobsList.push_back(newjob);

}
void JobsList::printJobsList()
{
    removeFinishedJobs();
    for (int i=0; i<(int)jobsList.size();++i)
    {
        if (jobsList[i]->status==FOREGROUND)
        {
            continue;
        }
        cout<<"["<<jobsList[i]->job_id<<"]"<<" "<<jobsList[i]->cmd<<endl;
    }
}

void JobsList::killAllJobs()
{
    removeFinishedJobs();
    cout << "smash: sending SIGKILL signal to " << (int)jobsList.size() << " jobs:" << endl;
    for (int i = 0; i < (int)jobsList.size(); ++i) {
        cout << jobsList[i]->pid << ": " << jobsList[i]->cmd << endl;
        DO_SYS(kill(jobsList[i]->pid, SIGKILL), kill);
    }
}

void JobsList::removeFinishedJobs() {
    vector<shared_ptr<JobEntry>> newJobList;


    for(int job=0;job<(int)jobsList.size();job++)
    {
        int sys_ret=0;
        DO_SYS(sys_ret=waitpid(jobsList[job]->pid,NULL,WNOHANG),waitpid);
        if(sys_ret==0)
        {
            newJobList.push_back(jobsList[job]);
        }
    }
    jobsList=newJobList;
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
    if(!jobsList.empty())
    {
        for (int i=0;i<(int)jobsList.size();i++)
        {
            if(jobsList[i]->job_id==jobId)
            {
                return jobsList[i].get();
            }
        }
    }
    return nullptr;
}

void JobsList::removeJobById(int jobId)
{
    for(int job=0;job<(int)jobsList.size();job++)
    {
        if(jobsList[job]->job_id==jobId)
        {
            jobsList[job]->status = FOREGROUND;
            jobsList.erase(job+jobsList.begin());
        }
    }
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId)
{
    if ((int)jobsList.size() == 0) {
        *lastJobId = 0;
        return nullptr;
    }
    *lastJobId = jobsList[jobsList.size() - 1]->job_id;
    return jobsList[jobsList.size() - 1].get();
}


//------------ Kill Command ------------//
void KillCommand::execute()
{
    string command = _trim(cmd_line);
    string params, first_string;
    string second_string;

    char* parsed[COMMAND_MAX_ARGS + 1];
    int num_of_args = _parseCommandLine(command.c_str(), parsed);


    if (command.find_first_of(" \n") == string::npos)
    {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    else
    {
        params = _trim(command.substr(command.find_first_of(" \n")));
        first_string = params.substr(0, 1);
    }

    if (first_string == "-")
    {
        params=_trim(params.substr(1));
        second_string = params.substr(0, params.find_first_of(" \n"));
    }
    int job_id = 0;
    try {
        params = _trim(params.substr(params.find_first_of(" \n")));
        string id_str = params.substr(0, params.find_first_of(" \n"));

        job_id = stoi(id_str);
    } catch (...) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }

    JobsList::JobEntry *job = jobs->getJobById(job_id);

    if (job== nullptr)
    {
        cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
        return;
    }
    if (first_string != "-")
    {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    if (num_of_args > 3) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    int signal = 0;
    //check if signal is a valid number
    try {
        signal = stoi(second_string);
    } catch (...) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    //check for legal signal
    if (signal < 1 || signal > 31) {
        cerr << "smash error: kill failed: Invalid argument" << endl;
        return;
    }

    DO_SYS(kill(job->pid, signal), kill);
    cout << "signal number " << signal << " was sent to pid " << job->pid << endl;

}

///----َQuit Command----////
void QuitCommand::execute()
{
    string command = _trim(cmd_line);
    string params , first_string;
    if (command.find_first_of(" \n") != string::npos) {
        params = _trim(command.substr(command.find_first_of(" \n")));
        first_string = params.substr(0, params.find_first_of(" \n"));
    }
    if (first_string == "kill")
    {
        jobs->killAllJobs();
    }
    exit(0);
}

////----ForeGround Command ----////
void ForegroundCommand::execute()
{
    string command = _trim(cmd_line);
    string params ,first_string;
    SmallShell &shell = SmallShell::getInstance();

    if (command.find_first_of(" \n") != string::npos)
    {
        params = _trim(command.substr(command.find_first_of(" \n")));
        first_string = params.substr(0, params.find_first_of(" \n"));
    }
    int job_id;

    if (first_string == "") {
        jobs->getLastJob(&job_id);

        if (job_id == 0) {
            cerr << "smash error: fg: jobs list is empty" << endl;
            return;
        }
    } else
    {
        try
        {
            job_id = stoi(first_string);
        } catch (std::exception &e)
        {
            cerr << "smash error: fg: invalid arguments" << endl;
            return;
        }
    }
    JobsList::JobEntry *job = jobs->getJobById(job_id);
    if (!job) {
        cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
        return;
    }
    char* parsed[COMMAND_MAX_ARGS + 1];
    int num_of_args = _parseCommandLine(command.c_str(), parsed);
    if(num_of_args>2)
    {
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }

    job->status = FOREGROUND;
    cout << job->cmd << " " << job->pid << endl;
    shell.running_cmd = job->cmd;
    shell.running_pid = job->pid;
    shell.running_id = job->job_id;
    jobs->removeJobById(job_id);
    DO_SYS(waitpid(job->pid, NULL, WUNTRACED), waitpid);
    shell.running_pid = -1;
    shell.running_id = -1;
    shell.running_cmd = "";

}

// ---------- PWD class ----------//
void GetCurrDirCommand::execute() {
    char cwd[MAXROUTE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        cout << cwd << endl;
    } else {
        perror("smash error: getcwd failed");
    }
}

/////pid function////
void ShowPidCommand::execute() {
    cout << "smash pid is " << SmallShell::getInstance().getShell_pid() << endl;
}


/////////cd function////////
void ChangeDirCommand::execute() {
    SmallShell &shell = SmallShell::getInstance();
    string new_dir;
    string command = this->cmd_line;
    command = _trim(command);
    // if (cd) with no parameters
    if ((int) command.length() == 3) {
        cerr << "smash error:>\"cd\"";
        return;
    }
    string params_only = command.substr(command.find_first_of(" \n"));
    params_only = _trim(params_only);
    string first_param = params_only.substr(0, params_only.find_first_of(" \n"));
    //if more than 1 params
    if (params_only.find_first_of(" \n") != string::npos) {
        cerr << "smash error: cd: too many arguments" << endl;
        return;
    }
    if (first_param == "-") {
        if ("" == shell.getPrev_dir()) {
            cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        } else {
            new_dir = shell.getPrev_dir();
        }
    } else {
        new_dir = first_param;
    }
    char curr_dir[MAXROUTE];
    if (getcwd(curr_dir, sizeof(curr_dir)) == NULL) {
        perror("smash error: getcwd failed");
    }
    int sys_ret;
    DO_SYS(sys_ret = chdir(new_dir.c_str()), chdir);
    if (sys_ret != -1) {
        shell.setPrev_dir(curr_dir);
    }
}
void JobsCommand::execute() {
    jobsList->printJobsList();

}
////////Redirection//////
void RedirectionCommand::execute() {

    string fullCMD= cmd_line;
    fullCMD= _trim(fullCMD);
    string cmd=fullCMD.substr(0,fullCMD.find_first_of(" \n"));
    cmd=_trim(cmd);
    string fileName=fullCMD.substr(fullCMD.find_last_of('>')+1);
    fileName=_trim(fileName);
    int stdOut;
    DO_SYS(stdOut = dup(STDOUT_FILENO), dup);
    DO_SYS(close(STDOUT_FILENO), close);
    int openD;
    if(fullCMD.find(">>")!=-1)//append

    {
        openD = open(fileName.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0655);
    }
    else
    {
        openD = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);

    }
    if(openD==-1)
    {
        perror("smash error: open failed");
        DO_SYS(dup2(stdOut, STDOUT_FILENO), dup2);
        DO_SYS(close(stdOut), close);
        return;

    }
    SmallShell::getInstance().executeCommand(cmd.c_str());
    DO_SYS(close(openD), open);
    DO_SYS(dup2(stdOut, STDOUT_FILENO), dup2);
    DO_SYS(close(stdOut), close);
    return;

}


////----External Commands----////
void ExternalCommand::execute()
{
    char cmd[COMMAND_ARGS_MAX_LENGTH];
    strcpy(cmd, cmd_line.c_str());
    bool is_bg = _isBackgroundComamnd(cmd);
    SmallShell &smash = SmallShell::getInstance();
    char *parsed[COMMAND_MAX_ARGS + 1];
    int num_of_args = _parseCommandLine(cmd_line.c_str(), parsed);

    if (is_bg)
    {
        _removeBackgroundSign(cmd);
        _parseCommandLine(cmd, parsed);
    }
    _parseCommandLine(cmd, parsed);
    int pid;
    DO_SYS(pid = fork(), fork);
    if (pid == -1)
    {
        //fork didn't work
        return;
    }
    if (pid ==0)
    {
        DO_SYS(setpgrp(), setpgrp);
        if (cmd_line.find('*') == string::npos && cmd_line.find('?') == string::npos)
        {
            // execute simple command
            if(execvp(parsed[0], parsed)==-1)
            {
                perror("smash error: execvp failed");
                exit(1);
            }
        }
        else
        {
            //Replace the current process with a new Bash shell executing the specified command
            DO_SYS(execl("/bin/bash", "/bin/bash", "-c", cmd, nullptr), execl);
        }
    }
    else
    {

        if (is_bg)
        {
            //add background job to Joblist
            smash.getJobsList()->addJob(this , pid , BACKGROUND);
        }
        else
        {
            int status;
            smash.running_pid = pid;
            smash.running_cmd = cmd_line;
            DO_SYS(waitpid(pid, &status, WUNTRACED), waitpid);
            smash.running_pid = -1;
            smash.running_id = -1;
            smash.running_cmd = "";
        }
    }
}
///////Chmod Command//////////
void ChmodCommand::execute()
{
    string command = _trim(cmd_line);
    string params_only, new_mode;
    char* parsed[COMMAND_MAX_ARGS + 1];
    int num_of_args = _parseCommandLine(command.c_str(), parsed);

    if (command.find_first_of(" \n") != string::npos) {
        params_only = _trim(command.substr(command.find_first_of(" \n")));
        new_mode = params_only.substr(0, 1);
    }
    if (new_mode != "-")
    {
        cerr << "smash error: gettype: invalid arguments" << endl;
        return;
    }
    if (num_of_args > 3) {
        cerr << "smash error: gettype: invalid arguments" << endl;
        return;
    }
    if (params_only.find_first_of(" \n") == string::npos) {
        cerr << "smash error: gettype: invalid arguments" << endl;
        return;
    }
    char*mode =new char[new_mode.size()+1];
    strcpy(mode,new_mode.c_str());
    mode_t newMode= strtol(mode, nullptr,8);
    params_only = _trim(params_only.substr(1));
    string file_path = params_only.substr(0, params_only.find_first_of(" \n"));
    if ( (newMode < 0) || (newMode >0777) )
    {
        cerr << "smash error: gettype: invalid arguments" << endl;
        return;
    }
    char* path=new char[file_path.size()+1];
    strcpy(path,file_path.c_str());

    DO_SYS(chmod(path,newMode),chmod);


}

void ChpromptCommand::execute() {
    SmallShell &smash = SmallShell::getInstance();
    string first_param;
    string cmd_s = _trim(string(cmd_line));

    if (cmd_s.find_first_of(" \n") == string::npos) {
        first_param = "smash";
    } else {
        string params_only = _trim(cmd_s.substr(cmd_s.find_first_of(" \n")));
        first_param = params_only.substr(0, params_only.find_first_of(" \n"));
    }
    smash.setShell_Prompt(first_param);
}

void PipeCommand::execute() {


    string redirect_cmd = cmd_line;
    redirect_cmd = _trim(redirect_cmd);
    string cmd1 = "";
    string cmd2 = "";
    bool is_pipe_err = redirect_cmd.find("|&") != string::npos;
    cmd1 = _trim(redirect_cmd.substr(0, redirect_cmd.find_first_of("|")));
    if (is_pipe_err)
    {
        cmd2 = redirect_cmd.substr(redirect_cmd.find_first_of("|&") + 2);
    }
    else
    {
        cmd2 = redirect_cmd.substr(redirect_cmd.find_first_of('|') + 1);
    }

    int pipe_arg[2], flag;
    DO_SYS(flag = pipe(pipe_arg), pipe);

    // pipe_arg[0] = read, pipe_arg[1] = write.
    int pid1, pid2;
    if(is_pipe_err)
    {
        DO_SYS(pid1 = fork() , fork);
        if(pid1 == 0)
        {
            DO_SYS(setpgrp() , setpgrp);
            DO_SYS(dup2(pipe_arg[1] , STDERR_FILENO) , dup2);
            DO_SYS(close(pipe_arg[0]) , close);
            DO_SYS(close(pipe_arg[1]) , close);
            SmallShell::getInstance().executeCommand(cmd1.c_str());
            exit(0);
        }
        DO_SYS(pid2 = fork() , fork);
        if(pid2 == 0)
        {
            DO_SYS(setpgrp() , setpgrp);
            DO_SYS(dup2(pipe_arg[0] , STDIN_FILENO) , dup2);
            DO_SYS(close(pipe_arg[0]) , close);
            DO_SYS(close(pipe_arg[1]) , close);
            SmallShell::getInstance().executeCommand(cmd2.c_str());
            exit(0);
        }
    }
    else
    {
        DO_SYS(pid1 = fork() , fork);
        if(pid1 == 0)
        {
            DO_SYS(setpgrp() , setpgrp);
            DO_SYS(dup2(pipe_arg[1] , STDOUT_FILENO) , dup2);
            DO_SYS(close(pipe_arg[0]) , close);
            DO_SYS(close(pipe_arg[1]) , close);
            SmallShell::getInstance().executeCommand(cmd1.c_str());
            exit(0);
        }
        DO_SYS(pid2 = fork() , fork);
        if(pid2 == 0)
        {
            DO_SYS(setpgrp() , setpgrp);
            DO_SYS(dup2(pipe_arg[0] , STDIN_FILENO) , dup2);
            DO_SYS(close(pipe_arg[0]) , close);
            DO_SYS(close(pipe_arg[1]) , close);
            SmallShell::getInstance().executeCommand(cmd2.c_str());
            exit(0);
        }
    }
    DO_SYS(close(pipe_arg[0]) , close);
    DO_SYS(close(pipe_arg[1]) , close);

    DO_SYS(waitpid(pid1 , NULL , 0) , waitpid);
    DO_SYS(waitpid(pid2 , NULL , 0) , waitpid);

}
