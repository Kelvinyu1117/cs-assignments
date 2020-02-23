#include <iostream>
#include <list>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>

using namespace std;

struct ProcessBlock
{
    pid_t pid;
    int status;
    string fileName;
    int state; // running: 0, stopping: 1, terminated: -1
};

list<ProcessBlock> processList;

unsigned int numsOfRunningProcess = 0;
unsigned int numsOfTerminatedProcess = 0;
unsigned int numsOfStoppedProcess = 0;

void bgCreate(char *fileArgs[])
{

    ProcessBlock pb;
    pb.fileName = fileArgs[0];

    pb.pid = fork();
    if (pb.pid == 0)
    { // child
        if (execvp(fileArgs[0], fileArgs) < 0)
        {
            cout << "execvp is failed for process." << endl;
        }
    }
    else if (pb.pid < 0) // error
    {
        cout << "fork error: " << pb.pid << endl;
        exit(EXIT_FAILURE);
    }

    if (numsOfRunningProcess >= 3) // pid > 0 -> parent
    {
        int res = kill(pb.pid, SIGSTOP);

        if (!res)
        {
            pb.state = 1;
            numsOfStoppedProcess++;

            processList.push_back(pb);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        pb.state = 0;
        numsOfRunningProcess++;

        processList.push_back(pb);
    }
}

void checkProcessStatus()
{
    int n = processList.size();
    for (ProcessBlock &pb : processList)
    {
        int waitRes = waitpid(pb.pid, &pb.status, WNOHANG);
        if (waitRes == pb.pid)
        {
            if (WIFEXITED(pb.status) && pb.state != -1)
            {
                pb.state = -1;
                cout << pb.pid << " completed" << endl;
                numsOfRunningProcess--;
                numsOfTerminatedProcess++;
            }
        }

        if (numsOfRunningProcess < 3)
        {
            if (pb.state == 1)
            {
                pb.state = 0;
                cout << pb.pid << " is automatically restarted" << endl;
                kill(pb.pid, SIGCONT);
                numsOfRunningProcess++;
            }
        }
    }
}

void bgList()
{
    for (ProcessBlock pb : processList)
    {
        cout << pb.pid << ": " << pb.fileName.substr(2);
        if (pb.state == -1)
        {
            cout << "(terminated)" << endl;
        }
        else if (pb.state == 0)
        {

            cout << "(running)" << endl;
        }
        else if (pb.state == 1)
        {
            cout << "(stopped)" << endl;
        }
    }
}

void bgStop(pid_t pid)
{

    list<ProcessBlock>::iterator it = processList.begin();

    for (; it != processList.end(); ++it)
    {
        if (it->pid == pid)
            break;
    }

    int res = kill(pid, SIGSTOP);

    if (!res)
    {
        it->state = 1;
        numsOfStoppedProcess++;
        numsOfRunningProcess--;
        
        cout << it->pid << " is stopped" << endl;

        ProcessBlock pb = *it;
        processList.erase(it);
        processList.push_back(pb);
    }
    else
    {
        exit(EXIT_FAILURE);
    }
}

void bgKill(pid_t pid)
{
    list<ProcessBlock>::iterator it2 = processList.begin();
    for (; it2 != processList.end(); ++it2)
    {
        if (it2->pid == pid)
            break;
    }

    int res = kill(pid, SIGSTOP);
    if (!res)
    {
        it2->state = -1;
        numsOfStoppedProcess++;
        numsOfRunningProcess--;
        cout << it2->pid << " skilled" << endl;
    }
}

void bgExit()
{
    for (ProcessBlock &pb : processList)
    {
        if (pb.state != -1)
        {
            int res = kill(pb.pid, SIGTERM);

            if (!res)
            {
                if (pb.state == 1)
                    numsOfStoppedProcess--;
                else
                    numsOfRunningProcess--;

                pb.state = -1;

                cout << pb.pid << " killed" << endl;
            }
        }
    }
}

int main()
{

    bool hasExit = false;
    do
    {
        checkProcessStatus();

        string s;
        cout << "BP >";
        getline(cin, s);

        char *p = strtok((char *)s.c_str(), " \t");

        if (p)
        {
            int pos = 0;
            char *command = p;
            char *fileArgs[5] = {NULL, NULL, NULL, NULL, NULL};

            while (p = strtok(NULL, " \t"))
            {
                fileArgs[pos++] = p;
            }

            if (!strcmp(command, "bg"))
            {
                string fileName = "./";
                fileName += fileArgs[0];
                fileArgs[0] = (char *)fileName.c_str();

                bgCreate(fileArgs);
            }
            else if (!strcmp(command, "bglist"))
            {
                bgList();
            }
            else if (!strcmp(command, "bgstop"))
            {
                bgStop(atoi(fileArgs[0]));
            }
            else if (!strcmp(command, "bgkill"))
            {
                bgKill(atoi(fileArgs[0]));
            }
            else if (!strcmp(command, "exit"))
            {
                bgExit();
                hasExit = true;
            }
        }
    } while (!hasExit);

    return 0;
}