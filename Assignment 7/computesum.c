#include "foothread.h"
// program to compute the sum of nodes in a tree

int n;

int P[100];
int C[100] = {0};
int S[100] = {0};
foothread_barrier_t barrier[100];
foothread_mutex_t mutex;

int compute_sum(void *arg)
{
    // get the id of the thread
    foothread_mutex_lock(&mutex);
    int *ptid = (int *)arg;
    int id = *ptid;

    // thread info
    // printf("thread: %02d tid: %d pid: %d\n", id, gettid(), getpid());
    
    // check if the node is a leaf
    if (C[id] == 0)
    {
        // it is a leaf node, ask the user for the value
        printf("Leaf node %3d :: Enter a positive integer: ", id);
        scanf("%d", &S[id]);
        foothread_mutex_unlock(&mutex);
        // wait for the other threads to finish
        foothread_barrier_wait(&barrier[id]);
    }
    else
    {
        foothread_mutex_unlock(&mutex);
        // it is not a leaf node
        // wait for the children to finish, then compute the sum
        foothread_barrier_wait(&barrier[id]);
        foothread_mutex_lock(&mutex);
        for (int i = 0; i < n; i++)
        {
            if (P[i] == id && i != id)
            {
                S[id] += S[i];
            }
        }
        printf("Internal node %3d gets the partial sum %d from its children\n", id, S[id]);
        foothread_mutex_unlock(&mutex);
    }
    // wait for the parent to finish
    foothread_barrier_wait(&barrier[P[id]]);

    foothread_exit();
    return 0;
}

int main()
{
    // read tree.txt file
    FILE *fp = fopen("tree.txt", "r");

    if (fp == NULL)
    {
        perror("File opening failed\n");
        exit(1);
    }

    // read the number of nodes
    fscanf(fp, "%d", &n);

    // create an array to store the parent of each node
    // int *P = (int *)malloc(n * sizeof(int));

    // read the parent of each node
    for (int i = 0; i < n; i++)
    {
        int node, parent;
        fscanf(fp, "%d %d", &node, &parent);
        P[node] = parent;
        C[parent]++;
        if (node == parent) C[parent]--;
        // printf("%d %d\n", node, parent);
    }

    for (int i = 0; i < n; i++)
    {
        foothread_barrier_init(&barrier[i], C[i]+1);
    }
    foothread_mutex_init(&mutex);

    // create synchronization variables
    // mutex

    // barrier

    // create n threads
    for (int i = 0; i < n; i++)
    {
        foothread_t thread;
        foothread_attr_t attr = FOOTHREAD_ATTR_INITIALIZER;
        foothread_attr_setjointype(&attr, FOOTHREAD_JOINABLE);
        int *arg = (int *)malloc(sizeof(int));
        *arg = i;
        foothread_create(&thread, &attr, compute_sum, (void *)arg);
        usleep(100);
    }

    // printf("Sum: %d\n", sum);
    
    foothread_exit();
    sleep(10);

    // find the node whos parent is itself
    int root;
    for (int i = 0; i < n; i++)
    {
        if (P[i] == i)
        {
            root = i;
            break;
        }
    }

    printf("Sum at root (node %d) = %d\n", root, S[root]);

    return 0;

}