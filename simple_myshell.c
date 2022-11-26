#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/wait.h>
#include "command.h"

#define MAX_CMD_ARG 10
#define BUF_SIZE 256

const char *prompt = "myshell> ";
char *cmdvector[MAX_CMD_ARG];
char cmdline[BUF_SIZE];
int redirect_input = 0;
int redirect_output = 1;
static sigjmp_buf jmpbuf;
static volatile sig_atomic_t canjump;

void setIgnoreHandler() {
    static struct sigaction act;

    void ignoreIntQuit(int);

    act.sa_handler = ignoreIntQuit;

    sigfillset(&(act.sa_mask));

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);
}

void setWaitHandler() {
    static struct sigaction wait;

    void waitChild(int);

    wait.sa_handler = waitChild;

    wait.sa_flags = SA_RESTART;

    sigfillset(&(wait.sa_mask));

    sigaction(SIGCHLD, &wait, NULL);
}

int makelist(char *s, const char *delimiters, char **list, int MAX_LIST) {
    int i = 0;
    int numtokens = 0;
    char *snew = NULL;

    if ((s == NULL) || (delimiters == NULL)) return -1;

    snew = s + strspn(s, delimiters);    /* Skip delimiters */
    if ((list[numtokens] = strtok(snew, delimiters)) == NULL)
        return numtokens;

    numtokens = 1;

    while (1) {
        if ((list[numtokens] = strtok(NULL, delimiters)) == NULL)
            break;
        if (numtokens == (MAX_LIST - 1)) return -1;
        numtokens++;
    }
    return numtokens;
}

int main(int argc, char **argv) {
    setIgnoreHandler();
    setWaitHandler();

    int type;
    while (1) {
        redirect_input = 0;
        redirect_output = 1;
        sigsetjmp(jmpbuf, 1);
        canjump = 1;
        memchr(cmdline, (char *) 0, BUF_SIZE);

        fputs(prompt, stdout);
        fgets(cmdline, BUF_SIZE, stdin);
        cmdline[strlen(cmdline) - 1] = '\0';
        int arguments_count = makelist(cmdline, " \t", cmdvector, MAX_CMD_ARG);

        if (!arguments_count) continue;

        if (is_builtin(cmdvector) > 0) continue;

        type = type_check(arguments_count, cmdvector);

        redirect_resolve(cmdvector, arguments_count, &redirect_input, &redirect_output);

        do_process(type, cmdvector, redirect_input, redirect_output);
    }
    return 0;
}

void ignoreIntQuit(int signo) {
    if (canjump == 0) return;
    puts("");
    canjump = 0;
    siglongjmp(jmpbuf, 1);
}

void waitChild(int signo) {
    pid_t pid = waitpid(-1, NULL, 0);
//    printf("background ended, pid: %d\n", pid);
}

