#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    SmallShell& smash = SmallShell::getInstance();
    cout << "smash: got ctrl-C" << endl;
    if(smash.running_pid == -1)
    {
        return;
    }
    int sys_result;
    DO_SYS(sys_result = kill(smash.running_pid , SIGKILL) , kill);
    if(sys_result == -1)
    {
        return;
    }
    cout << "smash: process " << smash.running_pid << " was killed" << endl;;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

