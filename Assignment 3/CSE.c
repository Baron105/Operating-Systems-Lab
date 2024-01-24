#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        // supervisor
        printf("+++ CSE in supervisor mode: Started\n");

        // pipes!
        int p1[2], p2[2];
        // p1 for communication between C and E, p2 for communication between E and C
        // make the pipe descriptors
        pipe(p1);
        pipe(p2);
        printf("+++ CSE in supervisor mode: pfd = [%d %d]\n", p1[0], p1[1]);

        // buffer for passing info and converting to string
        char arg1[5], arg2[5], arg3[5], arg4[5];
        sprintf(arg1, "%d", p1[0]);
        sprintf(arg2, "%d", p1[1]);
        sprintf(arg3, "%d", p2[0]);
        sprintf(arg4, "%d", p2[1]);

        // print the forking info
        printf("+++ CSE in supervisor mode: Forking first child in command-input mode\n");

        // fork the first child
        int pid = fork();
        if (pid == 0)
        {
            // command mode
            // run ./CSE C p1[0] p1[1] p2[0] p2[1]
            execlp("xterm", "xterm", "-T", "First Child", "-e", "./CSE", "C", arg1, arg2, arg3, arg4, NULL);
        }
        else
        {
            // print the forking info
            printf("+++ CSE in supervisor mode: Forking second child in execute mode\n");

            // fork the second child
            pid = fork();
            if (pid == 0)
            {
                // execute mode
                // run ./CSE E p1[0] p1[1] p2[0] p2[1]
                execlp("xterm", "xterm", "-T", "Second Child", "-e", "./CSE", "E", arg1, arg2, arg3, arg4, NULL);
            }
            else
            {
                // supervisor mode
                // close the pipe descriptors
                // wait for the children to finish
                wait(NULL);
                printf("+++ CSE in supervisor mode: First child terminated\n");
                wait(NULL);
                printf("+++ CSE in supervisor mode: Second child terminated\n");
                // print the forking info
            }
        }
    }

    else
    {
        // original stdin and stdout descriptors
        int outd = dup(1);
        int ind = dup(0);

        // set the pipe descriptors as inherited from the parent
        int p1[2], p2[2];
        p1[0] = atoi(argv[2]);
        p1[1] = atoi(argv[3]);
        p2[0] = atoi(argv[4]);
        p2[1] = atoi(argv[5]);

        // do separate things for C and E based on the mode md
        char md = argv[1][0];

        while (1)
        {
            if (md == 'C')
            {
                // C child
                // read user commands from stdin and send to E child via pipe

                // its stdout is p1[1]
                close(1);
                dup(p1[1]);

                // issue the prompt
                fprintf(stderr, "Enter command> ");

                char cmd[256];
                scanf("\n%[^\n]s", cmd);
                write(p1[1], cmd, strlen(cmd)+1);

                // if the command is exit, exit
                if (strcmp(cmd, "exit") == 0)
                {
                    exit(0);
                }

                // if the command is swaprole, swap the roles of the two processes
                if (strcmp(cmd, "swaprole") == 0)
                {
                    // swapping descriptors and changing mode
                    int temp = p1[0];
                    p1[0] = p2[0];
                    p2[0] = temp;

                    temp = p1[1];
                    p1[1] = p2[1];
                    p2[1] = temp;

                    md = 'E';
                }
            }

            else if (md == 'E')
            {
                // E child
                // reads command from pipe, forks a grandchild for executing command and waits till grandchild terminates
                // grandchild prints output to stdout of E

                // its stdin is p1[0]
                close(0);
                dup(p1[0]);

                // issue the prompt
                fprintf(stderr, "Waiting for command> ");

                char cmd[256];
                read(p1[0], cmd, 256);

                // if the command is exit, exit
                if (strcmp(cmd, "exit") == 0)
                {
                    exit(0);
                }

                // fork a grandchild to execute the commands
                int pid = fork();
                if (pid == 0)
                {
                    // grandchild
                    // restore the original stdin and stdout
                    dup2(ind, 0);
                    dup2(outd, 1);

                    // print the command
                    printf("%s\n", cmd);

                    // if the command is swaprole, swap the roles of the two processes
                    if (strcmp(cmd, "swaprole") == 0)
                    {
                        // swapping descriptors and changing mode
                        int temp = p1[0];
                        p1[0] = p2[0];
                        p2[0] = temp;

                        temp = p1[1];
                        p1[1] = p2[1];
                        p2[1] = temp;

                        md = 'C';
                    }

                    else
                    {
                        // execute the command
                        // first split into an array of strings

                        char *args[256];
                        char *ch = strtok(cmd, " ");
                        int c = 0;
                        while (ch != NULL)
                        {
                            args[c++] = ch;
                            ch = strtok(NULL, " ");
                        }
                        args[c] = NULL;

                        execvp(args[0], args);
                    }
                    // flush the output buffer
                    fflush(stdout);
                }
                else
                {
                    // E child
                    // wait for the grandchild to finish
                    wait(NULL);
                }
            }

            else
            {
                printf("Error: Mode not recognized\n");
                exit(1);
            }
        }
    }
}