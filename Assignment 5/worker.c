#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#define P(s) semop(s, &pop, 1)  // P(s) or wait(s)
#define V(s) semop(s, &vop, 1)  // V(s) or signal(s)

int main(int argc, char *argv[])
{
    // check if there are two arguments n and i
    if (argc != 3)
    {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    // get n and i from arguments
    int n = atoi(argv[1]);
    int i = atoi(argv[2]);

    // get key
    key_t keyA = ftok("graph.txt", 1);
    key_t keyT = ftok("graph.txt", 2);
    key_t keyidx = ftok("graph.txt", 3);

    // get shmidA, shmidT, shmididx
    int shmidA = shmget(keyA, n * n * sizeof(int), 0666);
    int shmidT = shmget(keyT, n * sizeof(int), 0666);
    int shmididx = shmget(keyidx, sizeof(int), 0666);

    // get A, T, idx
    int *A = (int *)shmat(shmidA, 0, 0);
    int *T = (int *)shmat(shmidT, 0, 0);
    int *idx = (int *)shmat(shmididx, 0, 0);

    // get keymtx, keysync, keyntfy
    key_t keymtx = ftok("graph.txt", 4);
    key_t keysync = ftok("graph.txt", 5);
    key_t keyntfy = ftok("graph.txt", 6);

    // get mtx, syncid, ntfy
    int mtx = semget(keymtx, 1, 0666);
    int syncid = semget(keysync, n, 0666);
    int ntfy = semget(keyntfy, 1, 0666);

    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = i;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;

    // wait for sync signals from all incoming links(j, i)
    for (int j = 0; j < n; j++)
    {
        if (A[j * n + i] == 1)
        {
            P(syncid);
        }
    }

    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;

    // critical section
    P(mtx);
    T[*idx] = i;
    *idx = *idx + 1;
    // printf("idx: %d i: %d\n", *idx, i);
    V(mtx);

    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;

    // signal sync signals to all outgoing links(i, j)
    for (int j = 0; j < n; j++)
    {
        pop.sem_num = vop.sem_num = j;
        if (A[i * n + j] == 1)
        {
            V(syncid);
        }
    }

    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;

    // signal ntfy to notify the boss that worker i has finished
    V(ntfy);

    return 0;
}