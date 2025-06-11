#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <memory>
#include <stack>
#include <sys/stat.h>



using std::string;
using std::shared_ptr;
using std::vector;
#define COMMAND_ARGS_MAX_LENGTH (80)
#define COMMAND_MAX_ARGS (20)
#define MAXROUTE 80
#define DO_SYS( syscall, name ) do { \
    if( (syscall) == -1 ) { \
      perror("smash error: " #name " failed"); \
      return; \
    }         \
  } while(0)  \


class Command {
// TODO: Add your data members
protected:
    string cmd_line;
 public:
  Command(const char* cmd_line):cmd_line(cmd_line){}
  virtual ~Command() = default;
  virtual void execute() = 0;
    string getCMD(){return cmd_line;}
  // TODO: Add your extra methods if needed

};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line):Command(cmd_line){}
  virtual ~BuiltInCommand() {}

};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line):Command(cmd_line){}
  virtual ~ExternalCommand() {}
  void execute() override;

};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line):Command(cmd_line){}
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line):Command(cmd_line){};
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
public:
    // TODO: Add your data members public:
    explicit ChangeDirCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~ChangeDirCommand() {}
  void execute() override;

};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};
class ChpromptCommand : public BuiltInCommand {
public:
    ChpromptCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
    virtual ~ChpromptCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
    JobsList* jobs;

    virtual ~QuitCommand() {}
  void execute() override;

public:
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line),jobs(jobs){}
};



enum Status {BACKGROUND,FOREGROUND};


class JobsList {
 public:

  class JobEntry {
  public:
      int pid;
      int job_id;
      Status status;
      const string cmd;


      JobEntry(int pid,int job_id,Status status,const string &cmd):pid(pid),job_id(job_id),status(status)
      ,cmd(cmd){}

      // TODO: Add your data members
  };
  vector<shared_ptr<JobEntry>> jobsList;
 // TODO: Add your data member

public:
  JobsList()=default;
  ~JobsList()=default;
  void addJob(Command* cmd,int pid,Status status);

  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobsList;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line),jobsList(jobs)
  {
  }
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;
 public:
  KillCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line),jobs(jobs){}
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line),jobs(jobs){}
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand {
    JobsList* jobsList;
 public:
  ChmodCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
  virtual ~ChmodCommand() {}
  void execute() override;
};


class SmallShell {
 private:
    string shell_prompt;
    int shell_pid;
    string prev_dir;
    JobsList* jobs;
  // TODO: Add your data members
  SmallShell();
 public:
    std::stack<string> path;
    int running_id;
    int running_pid;
    string running_cmd;

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

  void setShell_Prompt(const string &str)
  {
      shell_prompt=str;
  }
  string getShelPrompt() const
  {
      return shell_prompt;
  }
  void setPrev_dir(const string &str)
  {
      prev_dir=str;
  }
  string getPrev_dir() const
  {
      return prev_dir;
  }
  int getShell_pid() const
  {
      return shell_pid;
  }
  JobsList* getJobsList()
  {
      return jobs;
  }
  // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
