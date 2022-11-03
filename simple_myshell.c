#include <string.h>
#include <stdio.h>
#include "command.h"

#define MAX_CMD_ARG 10
#define BUF_SIZE 256

const char *prompt = "myshell> ";
char *cmdvector[MAX_CMD_ARG];
char cmdline[BUF_SIZE];


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
    int type;
    while (1) {
        fputs(prompt, stdout);
        fgets(cmdline, BUF_SIZE, stdin);
        cmdline[strlen(cmdline) - 1] = '\0';
        int arguments_count = makelist(cmdline, " \t", cmdvector, MAX_CMD_ARG);

        if(!arguments_count) continue;

        if (is_builtin(cmdvector) > 0) continue;

        type = type_check(arguments_count, cmdvector);
        do_process(type,cmdvector);
        memchr(cmdline, (char*)0, BUF_SIZE);
    }
    return 0;
}


