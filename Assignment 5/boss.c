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

int main()
{
    // read graph.txt first line to get the number of vertices
    FILE *f = fopen("graph.txt", "r");
    int n;
    struct sembuf pop, vop;
    fscanf(f, "%d", &n);

    // create shared adjacency matrix A to store the graph and T to store the topology
    key_t keyA = ftok("graph.txt", 1);
    key_t keyT = ftok("graph.txt", 2);
    key_t keyidx = ftok("graph.txt", 3);

    int shmidA = shmget(keyA, n * n * sizeof(int), 0666 | IPC_CREAT);
    int shmidT = shmget(keyT, n * sizeof(int), 0666 | IPC_CREAT);

    // also create idx to store the index of the next vertex to be processed
    int shmididx = shmget(keyidx, sizeof(int), 0666 | IPC_CREAT);

    int *A = (int *)shmat(shmidA, 0, 0);
    int *T = (int *)shmat(shmidT, 0, 0);
    int *idx = (int *)shmat(shmididx, 0, 0);

    // initialize T and idx
    for (int i = 0; i < n; i++)
    {
        T[i] = 0;
    }
    *idx = 0;

    key_t keymtx = ftok("graph.txt", 4);
    key_t keysync = ftok("graph.txt", 5);
    key_t keyntfy = ftok("graph.txt", 6);

    // create semaphore mtx to ensure mutual exclusion among workers
    int mtx = semget(keymtx, 1, 0666 | IPC_CREAT);
    semctl(mtx, 0, SETVAL, 1);

    // create semaphore set of n semaphores to ensure that each worker waits for its turn
    int syncid = semget(keysync, n, 0666 | IPC_CREAT);
    for (int i = 0; i < n; i++)
    {
        semctl(syncid, i, SETVAL, 0);
    }
    // sync[i] blocks worker wi until all wj with links (j, i) are processed

    // create semaphore ntfy to notify the boss that all workers have finished
    int ntfy = semget(keyntfy, 1, 0666 | IPC_CREAT);
    // ntfy blocks the boss until all workers have finished, set it to -(n-1) initially
    semctl(ntfy, 0, SETVAL, 0);


    // read graph.txt to fill A
    for (int i = 0; i < n * n; i++)
    {
        fscanf(f, "%d", &A[i]);
    }

    printf("+++ Boss: Setup done...\n");

    // wait for all workers to finish using ntfy
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;

    for (int i = 0; i < n; i++)
    {
        P(ntfy);
    }
    

    printf("+++ Topological sorting of the vertices\n");
    // print T
    for (int i = 0; i < n; i++)
    {
        printf("%d ", T[i]);
    }
    printf("\n+++ Boss: Well done, my team...\n");

    // detach and remove shared memory
    shmdt(A);
    shmdt(T);
    shmdt(idx);

    shmctl(shmidA, IPC_RMID, NULL);
    shmctl(shmidT, IPC_RMID, NULL);
    shmctl(shmididx, IPC_RMID, NULL);

    // remove semaphores
    semctl(mtx, 0, IPC_RMID, 0);
    semctl(syncid, 0, IPC_RMID, 0);
    semctl(ntfy, 0, IPC_RMID, 0);

    return 0;
}