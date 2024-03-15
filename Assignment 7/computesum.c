#include "foothread.h"
// program to compute the sum of nodes in a tree

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
    int n;
    fscanf(fp, "%d", &n);

    // create an array to store the parent of each node
    int *P = (int *)malloc(n * sizeof(int));

    // read the parent of each node
    for (int i = 0; i < n; i++)
    {
        int node, parent;
        fscanf(fp, "%d %d", &node, &parent);
        P[node] = parent;
        // printf("%d %d\n", node, parent);
    }

    

}