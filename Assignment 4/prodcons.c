#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main()
{
    int n, t;
    int *m;

    // ask for number of consumers ie child processes
    printf("n = ");
    scanf("%d", &n);

    // ask for number of items to be produced
    printf("t = ");
    scanf("%d", &t);

    // create shared memory for two integers, cid and item
    int shmid = shmget(IPC_PRIVATE, 2 * sizeof(int), 0777 | IPC_CREAT);

    // attach shared memory to parent process
    // m[0] = cid, m[1] = item
    m = (int *)shmat(shmid, 0, 0);

    // initialize cid and item
    m[0] = 0;
    m[1] = 0;

    // create n child processes
    for (int i = 1; i <= n; i++)
    {
        if (fork() == 0)
        {
            // child process
            // attach shared memory to child process
            long long csum = 0;
            int ct = 0;
            int *cm;
            m = (int *)shmat(shmid, 0, 0);

            printf("\t\t\t\tConsumer %d is alive\n", i);

            // check if m[0] is equal to i then read the item and set m[0] to 0
            // else wait for m[0] to be equal to i
            while (1)
            {
                // if m[0] is equal to -1 then exit
                if (m[0] == -1)
                {
                    break;
                }

                // if m[0] is equal to i then read the item and set m[0] to 0
                // also ensure that m[1] is not 0 as it means m[1] has not been written to yet
                if (m[0] == i && m[1] != 0)
                {
                    // verbose flag
                    #ifdef VERBOSE
                        printf("\t\t\t\tConsumer %d reads %d\n", i, m[1]);
                    #endif
                    csum += m[1];
                    ct++;
                    m[0] = 0;
                    m[1] = 0;
                }
            }

            // print consumer sum and count
            printf("\t\t\t\tConsumer %d has read %d items: Checksum = %lld\n", i, ct, csum);

            // detach shared memory from child process
            shmdt(m);

            // exit child process
            exit(0);
        }
    }

    // parent process
    printf("Producer is alive\n");

    // create two arrays to store the sum and count of each consumer
    long long psum[n];
    int pct[n];

    // initialize sum and count to 0
    for (int i = 0; i < n; i++)
    {
        psum[i] = 0;
        pct[i] = 0;
    }

    // producer loop

    for (int i = 0; i < t; i++)
    {
        // wait for m[0] to be 0
        while (m[0] != 0);

        // if m[0] is 0 then get a random number x between 1 and n
        m[0] = rand() % n + 1;
        int x = m[0];
        pct[x - 1]++;

        #ifdef SLEEP
            // sleep for 1 microsecond
            usleep(1);
        #endif

        // set m[1] to random 3 digit number
        m[1] = rand() % 900 + 100;
        psum[x - 1] += m[1];

        // print producer message
        #ifdef VERBOSE
            printf("Producer produces %d for Consumer %d\n", m[1], m[0]);
        #endif
    }

    // sleep for 1 second as a precaution
    sleep(1);

    // after all items are produced set m[0] to -1
    m[0] = -1;

    // wait for all child processes to exit
    for (int i = 1; i <= n+1; i++)
    {
        wait(NULL);
    }

    // print producer sum and count
    printf("Producer has produced %d items\n", t);

    // print consumer sum and count
    for (int i = 0; i < n; i++)
    {
        printf("%d items for Consumer %d: Checksum = %lld\n", pct[i], i + 1, psum[i]);
    }

    // detach shared memory from parent process
    shmdt(m);

    // delete shared memory
    shmctl(shmid, IPC_RMID, 0);

    return 0;
}