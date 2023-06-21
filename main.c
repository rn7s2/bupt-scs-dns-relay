#include <sys/wait.h>
#include <sys/errno.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    pid_t pid = fork();
    if (pid == 0) {
        printf("I am child process, pid = %d\n", getpid());
    } else if (pid > 0) {
        printf("I am parent process, pid = %d\n", getpid());
        pid_t child = waitpid(pid, NULL, 0);
        if (child == -1) {
            printf("waitpid failed, errno = %d\n", errno);
        } else {
            printf("child process %d exited\n", child);
        }
    } else {
        printf("fork failed\n");
    }
    return 0;
}
