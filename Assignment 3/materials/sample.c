#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    int child_pid;

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork a child process
    if ((child_pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        // Child process


        // Close the read end of the pipe, as it is not needed by the parent
        close(pipefd[0]);

        // Redirect stdout to write to the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // Execute "ls"
        execlp("ls", "ls", (char *)NULL);
        

        // If execlp fails
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process

        // Close the write end of the pipe, as it is not needed by the child
        close(pipefd[1]);

        // Redirect stdin to read from the pipe
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        // Execute "wc -l"
        execlp("wc", "wc", "-l", (char *)NULL);

        // If execlp fails
        perror("execlp");
        wait(NULL);
        exit(EXIT_FAILURE);
    }

    return 0;
}
