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
		char cityname[20] = {'\0'};
		int idx = 0;
		for (int j=0;j<20;j++)
		{
			if (line[j]=='\0'||line[j]=='\n'||line[j]==' ') break;
			cityname[j] = line[j];
		}
		fo = 1;
		for (int j=0;j<strlen(cityname);j++) {
			if (cityname[j]!=city[j]) fo = 0;
		}

		if (fo == 1)
		{
			// getting number of children
			char ch = line[l+1];
			int in = ch - 48;
			for (int j=0;j<s;j++) printf("    ");
			printf("%s (%d)\n", city, getpid());

			int id = l+2;
			int ind = in;
			// printf("%d\n", in);

			while(ind--)
			{
				int level = 0;
				char ncity[20]={'\0'};
				int idx=0;
				id++;
				// extract child city
				while (line[id]!='\0'&&line[id]!=' '&&line[id]!='\n') {
					ncity[idx] = line[id];
					idx++;
					id++;
				}

				// printf("%s\n", ncity);

				int pid = fork();

				if (pid == 0)
				{
					level++;
					// child
					char buf[100];

					// check if it is valid
					if (ncity[0] == '\0') {
						exit(0);
					}
					sprintf(buf, "%d", s+level);
					execl("./proctree", "./proctree", ncity, buf, NULL);
					exit(0);
				}
				else
				{
					wait(NULL);
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
