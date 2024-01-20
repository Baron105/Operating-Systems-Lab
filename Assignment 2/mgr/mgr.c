#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

// global array for process table
int pgida[11] = {0};
int pida[11] = {0};
int stat[11] = {0};
char name[11][6] = {'\0'};

// pid and to check length of table
int pid;
int j = 1;

// handlers for ctrl c ctrl z
void hand(int s)
{
    if (s == SIGINT) 
    {
        if (pid == pida[0])
            return;

        // handle sigint
        kill(pid, SIGINT);

        // change in table
        for (int i=0;i<j;i++)
        {
            if (pida[i]==pid) 
            {
                stat[i] = 4;
            }
        }

        pid = pida[0];
    }

    else if (s == SIGTSTP)
    {
        if (pid == pida[0]) return;

        // handle sigstop
        kill(pid, SIGTSTP);

        // change in table
        for (int i = 0; i < j; i++)
        {
            if (pida[i] == pid)
            {
                stat[i] = 3;
            }
        }

        pid = pida[0];
    }

}

int main()
{
    srand((unsigned int)time(NULL));
    pgida[0] = pida[0] = getpid();
    stat[0] = 0;
    strcpy(name[0],"mgr");
    char c;

    while(1)
    {
        
        printf("mgr > ");
        scanf("%c", &c);
        if (c=='h') // help
        {
            printf("Command : Action\nc : Continue a suspended job\nh : Print this help message\nk : Kill a suspended job\np : Print the process table\nq : Quit\nr : Run a new job\n");
            continue;
        }
        else if (c=='r') // run
        {
            // exit if already 10 processes
            if (j == 10)
            {
                printf("Process table is full. Quiting...");
            }

            // run job after forking
            char arg[1];
            arg[0] = 'A' + rand() % 26;
            pid = fork();
            if (pid == 0)
            {
                // child
                // pgida[j] = pida[j] = getpid();

                // change gpid
                setpgid(getpid(),0);
                execl("./job","./job", arg ,NULL); // execute
            }

            else{
                // parent
                pgida[j] = pida[j] = pid;
                stat[j] = 2;
                name[j][0] = arg[0];
                j++; 

                signal(SIGINT, hand);
                signal(SIGTSTP, hand);
                // handle separately

            }
        }
        else if (c=='q') // quit
        {
            // kill orphaned processes

            for (int i = 0; i < j; i++)
            {
                if (stat[i] == 3) kill(pida[i], SIGKILL);
            }

            exit(0);
        }
        else if (c=='k') // kill
        {
            int x = 0;
            for (int i = 0; i < j; i++)
            {
                if (stat[i] == 3) x = 1;
            }

            if (x == 1)
            {
                printf("Suspended jobs: ");
                for (int i = 0; i < j; i++)
                {
                    if (stat[i] == 3)
                        printf(" %d", i);
                }
                printf(" (Pick one): ");
                scanf("%d", &x);
                kill(pida[x], SIGKILL);
                pid = pida[x];
                stat[x] = 5;
            }
        }
        else if (c=='p') // print process table
        {
            printf("NO\tPID\tPGID\tSTATUS\t\tNAME\n");
            for (int i=0;i<j;i++)
            {
                printf("%d\t%d\t%d\t",i,pida[i],pgida[i]);
                if (stat[i]==0) printf("SELF      ");
                // else if (stat[i] == 1) printf("RUNNING   ");
                else if (stat[i] == 2) printf("FINISHED  ");
                else if (stat[i] == 3) printf("SUSPENDED" );
                else if (stat[i] == 4) printf("TERMINATED");
                else if (stat[i] == 5) printf("KILlED    ");
                if (i==0) printf("\tmgr\n");
                else printf("\tJob %s\n",name[i]);
            }
        }
        else if (c=='c') // continue
        {
            // suspended stat = 3
            int x = 0;
            for (int i = 0; i < j; i++)
            {
                if (stat[i]==3) x = 1; 
            }

            if (x==1)
            {
                printf("Suspended jobs: ");
                for (int i = 0; i < j; i++)
                {
                    if (stat[i] == 3) printf(" %d", i);
                }
                printf(" (Pick one): ");
                scanf("%d",&x);
                kill(pida[x], SIGCONT);
                pid = pida[x];
                stat[x] = 2;
            }
        }

    
    }
}