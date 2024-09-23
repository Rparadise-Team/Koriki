#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    // Create a new process using fork
    pid_t pid = fork();

    if (pid == -1) {
        // Error creating the process
        perror("Error creating process");
        return 1;
    } else if (pid == 0) {
        // Child process: Execute the command to run the Python script
        execlp("python", "python", "./keytester.py", (char *) NULL);

        // If execlp fails
        perror("Error executing Python script");
        return 1;
    } else {
        // Parent process: Wait for the child process (python ./keytester.py) to finish
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("keytester.py finished with status %d\n", WEXITSTATUS(status));
        } else {
            printf("keytester.py terminated abnormally\n");
        }

        // The binary closes after keytester.py finishes
        printf("Python script executed successfully\n");
    }

    return 0;
}
