#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main()
{
    int shmid;
    int key;

    key = ftok("p2.c", 'R');

    shmid = shmget(key, 256, 0777|IPC_CREAT);

    char* shared;

    shared = shmat(shmid, NULL, 0);

    char info[20];

    printf("Enter Text to Share: ");
    scanf("%[^\n]", info);

    strncpy(shared, info, 256);
    shmdt(shared);

    return 0;
}