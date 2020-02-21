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

void bgList();

void tmp_bgCreate(const char *fileArgs[])
{
    char *const *tmp = (char *const *)fileArgs;

    ProcessBlock pb;
    pb.fileName = fileArgs[0];

    pb.pid = fork();
    if (pb.pid == 0)
    { // child
        if (execvp(tmp[0], tmp) < 0)
        {
            cout << "execvp is failed for process." << endl;
        }
    }
    else if (pb.pid < 0) // error
    {
        cout << "fork error: " << pb.pid << endl;
        exit(EXIT_FAILURE);
    }

    if (numsOfRunningProcess < 3)
    {
        pb.state = 0;
        numsOfRunningProcess++;

        processList.push_back(pb);
    }
    else
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
}

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

    if (numsOfRunningProcess < 3) // pid > 0 -> parent
    {

        pb.state = 0;
        numsOfRunningProcess++;

        processList.push_back(pb);
    }
    else
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
}

void printProcessList()
{
    for (ProcessBlock pb : processList)
    {
        printf("{pid: %d, status: %d, fileName: %s}\n", pb.pid, pb.status, pb.fileName.c_str());
    }
    cout << endl;
}

void checkProcessStatus(bool &stop)
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

    if (n == numsOfTerminatedProcess)
        stop = true;
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

void test_bgCreate()
{
    const char *test1_arg[] = {"./demo1", "demo1", "1", "2", NULL};
    bool stop = false;

    // test 1
    tmp_bgCreate(test1_arg);
    cout << "Num of Running Process: " << numsOfRunningProcess << endl;
    bgList();

    // test 2
    const char *test2_arg[] = {"./demo2", "demo2", "2", "2", NULL};
    tmp_bgCreate(test2_arg);
    cout << "Num of Running Process: " << numsOfRunningProcess << endl;
    bgList();

    // test 3
    const char *test3_arg[] = {"./demo3", "demo3", "3", "2", NULL};
    tmp_bgCreate(test3_arg);
    cout << "Num of Running Process: " << numsOfRunningProcess << endl;
    bgList();

    // test 4
    const char *test4_arg[] = {"./demo4", "demo4", "4", "2", NULL};
    tmp_bgCreate(test4_arg);
    cout << "Num of Running Process: " << numsOfRunningProcess << endl;
    bgList();

    while (!stop)
    {
        checkProcessStatus(stop);
    }
}

void bgList()
{
    for (ProcessBlock pb : processList)
    {
        printf("%d: %s", pb.pid, pb.fileName.substr(2).c_str());
        if (pb.state == -1)
        {
            printf("(terminated)\n");
        }
        else if (pb.state == 0)
        {
            printf("(running)\n");
        }
        else if (pb.state == 1)
        {
            printf("(stopped)\n");
        }
    }
}

void test_bgList()
{
    const char *test1_arg[] = {"./demo1", "demo1", "1", "2", NULL};
    bool stop = false;

    // test 1
    tmp_bgCreate(test1_arg);
    cout << "Test 1" << endl;
    bgList();
    cout << endl;

    // test 2
    const char *test2_arg[] = {"./demo2", "demo2", "2", "2", NULL};
    tmp_bgCreate(test2_arg);
    cout << "Test 2" << endl;
    bgList();
    cout << endl;

    // test 3
    const char *test3_arg[] = {"./demo3", "demo3", "3", "2", NULL};
    tmp_bgCreate(test3_arg);
    cout << "Test 3" << endl;
    bgList();
    cout << endl;

    // test 4
    const char *test4_arg[] = {"./demo4", "demo4", "4", "2", NULL};
    tmp_bgCreate(test4_arg);
    cout << "Test 4" << endl;
    bgList();
    cout << endl;

    while (!stop)
    {
        checkProcessStatus(stop);
    }

    cout << "Test 5" << endl;
    bgList();
    cout << endl;
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
        printf("%d is stopped\n", it->pid);

        ProcessBlock pb = *it;
        processList.erase(it);
        processList.push_back(pb);
    }
    else
    {
        exit(EXIT_FAILURE);
    }
}

void test_bgStop()
{
    const char *test1_arg[] = {"./demo1", "demo1", "8", "3", NULL};
    bool stop = false;

    tmp_bgCreate(test1_arg);

    const char *test2_arg[] = {"./demo2", "demo2", "6", "4", NULL};
    tmp_bgCreate(test2_arg);

    const char *test3_arg[] = {"./demo3", "demo3", "6", "5", NULL};
    tmp_bgCreate(test3_arg);

    const char *test4_arg[] = {"./demo4", "demo4", "4", "2", NULL};
    tmp_bgCreate(test4_arg);

    int i = 0;
    bgList();
    cout << endl;

    list<ProcessBlock>::iterator it = processList.begin();

    while (!stop)
    {
        checkProcessStatus(stop);
        for (int j = 0; i < 2; i++) // stop 1 and 3
        {
            cout << "Test " << j++ << endl;
            list<ProcessBlock>::iterator it1 = it++;
            bgStop(it->pid);
            cout << endl;
            it = it1;
            bgList();
            cout << endl;
        }
    }
}

void bgKill(pid_t pid)
{
}

void test_bgKill()
{

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

                printf("%d is killed\n", pb.pid);
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
                cout << "bgkill" << endl;
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