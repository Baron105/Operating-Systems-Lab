// Barun Parua
// 21CS10014
// Assignment 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int s;
	char* city;
	FILE* f = fopen("treeinfo.txt", "r");
	if (argc == 1) {
		printf("Run with a node name\n");
		return 0;
	}
	else if (argc == 2) {
		s = 0;
		city = argv[1];
	}
	else {
		s = atoi(argv[2]);
		city = argv[1];
	}
	int l = strlen(city);
	char line[100];
	const char d[2] = " ";
	int fo = 1;
	for (int i=0;i<22;i++)
	{
		fgets(line, 100, f);
		char* token = strtok(line, d);
		fo = 1;
		for (int j=0;j<l;j++) {
			if (line[j]!=city[j]) fo = 0;
		}

		if (fo == 1)
		{
			// getting number of children
			char ch = line[l+1];
			int in = ch - 48;
			for (int j=0;j<s;j++) printf("\t");
			printf("%s (%d)\n", city, getpid());

			int id = l+3;
			int ind = in;

			while(ind--)
			{
				int pid = fork();
				char ncity[20]={'\0'};
				int idx=0;
				// extract child city
				while (line[id]!='\0'&&line[id]!=' '&&line[id]!='\n') {
					ncity[idx] = line[id];
					idx++;
					id++;
				}
				if (pid)
				{
					// child
					char buf[100];
					sprintf(buf, "%d", s+in-ind);
					execl("./proctree", "./proctree", ncity, buf, NULL);
					exit(0);
				}
				else
				{
					// parent
					while ((wait(NULL)) > 0);
					exit(0);
				}
			}

			return 0;
		}

		continue;

	}
	// if not found
	printf("City %s not found\n", city);
	return 0;
}
