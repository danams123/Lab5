#include "linux/limits.h"
#include <unistd.h>
#include <stdio.h>
#include "LineParser.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define INPUT_MAX_SIZE 48
#define EXECUTION_FAILED 1
#define BUFFER_SIZE 12

int debug = 0;
char buffer[BUFFER_SIZE];
/* returns the decimal representation of a _signed_ integer */
char *itoa(int num)
{

    char *p = buffer + BUFFER_SIZE - 1;
    int neg = num < 0;

    if (neg)
    {
        num = -num;
    }

    *p = '\0';
    do
    {
        *(--p) = '0' + num % 10;
    } while (num /= 10);

    if (neg)
    {
        *(--p) = '-';
    }

    return p;
}

void execute(cmdLine *pCmdLine)
{
    int returnVal;
    char *pid_str;
    int err = 0;
    pid_t pid = 0;
    int status;

    if (strncmp(pCmdLine->arguments[0], "cd", 2) == 0)
    {
        if (strncmp(pCmdLine->arguments[1], "..", 2) == 0)
        {
            err = chdir("..");
        }
        else
        {
            err = chdir(pCmdLine->arguments[1]);
        }
        if (err < 0)
        {
            write(STDERR_FILENO, "no such direcetory\n", 20);
        }
    }

    else if ((pid = fork()) == 0)
    {
        if ((returnVal = execv(pCmdLine->arguments[0], pCmdLine->arguments)) < 0)
        {
            perror("couln't execute");
            _exit(EXECUTION_FAILED);
        }
    }
    if (debug == 1)
    {
        write(STDERR_FILENO, "pid is: ", 9);
        pid_str = itoa(pid);
        write(STDERR_FILENO, pid_str, strlen(pid_str));
        write(STDERR_FILENO, "\n", 1);
        write(STDERR_FILENO, "Executing command is: ", 23);
        write(STDERR_FILENO, pCmdLine->arguments[0], strlen(pCmdLine->arguments[0]));
        write(STDERR_FILENO, "\n", 1);
    }
    if (pCmdLine->blocking == 1)
    {
        waitpid(pid, &status, WUNTRACED);
    }
}

int main(int argc, char **argv)
{
    char buf[PATH_MAX];
    char input[INPUT_MAX_SIZE];
    cmdLine *cmdL;
    int i;

    for (i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "-d", 2) == 0)
            debug = 1;
    }

    while (1)
    {
        char *returnVal = getcwd(buf, PATH_MAX);
        if (returnVal == NULL)
        {
            return 1;
        }

        fprintf(stdout, "%s\n", returnVal);

        fgets(input, INPUT_MAX_SIZE, stdin);
        // sscanf(input, "%s", command);

        if (strncmp(input, "quit", 4) == 0)
        {
            freeCmdLines(cmdL);
            break;
        }

        if ((cmdL = parseCmdLines(input)) == NULL)
        {
            fprintf(stdout, "%s", "nothing to parse\n");
            return 0;
        }
        execute(cmdL);
        freeCmdLines(cmdL);
    }
    return 0;
}