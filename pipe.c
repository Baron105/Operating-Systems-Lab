/*
** pipe2.c -- a smarter pipe example
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
	int pfds[2];
	char buf[30];

	pipe(pfds);
	printf("pfds[0] = %d\n", pfds[0]);
	printf("pfds[1] = %d\n", pfds[1]);
	pipe(pfds);
	printf("pfds[0] = %d\n", pfds[0]);
	printf("pfds[1] = %d\n", pfds[1]);

	if (!fork()) {
		printf(" CHILD: writing to the pipe\n");
		write(0, "test", 5);
		printf(" CHILD: exiting\n");
		dup(0);
		// read(pfds[0], buf, 5);
		// printf(" CHILD: read \"%s\"\n", buf);
		exit(0);
	} else {
		// sleep(1);
		printf("PARENT: reading from pipe\n");
		// This read blocks.  Fortunately, the child
		// writes to it in non-blocking mode and then closes it,
		// causing the OS trigger this process to wake up.
		read(1, buf, 5);
		printf("PARENT: read %s\n", buf);
		wait(NULL);
	}

	return 0;
}