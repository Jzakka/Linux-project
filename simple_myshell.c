#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_CMD_ARG 10
#define BUFSIZ 256

#define CD 0
#define PWD 1
#define EXIT 2

const char *prompt = "myshell> ";
char *cmdvector[MAX_CMD_ARG];
char cmdline[BUFSIZ];
char *commands[3] = {"cd", "pwd", "exit"};

int is_builtin();

void do_command(char *string, char *pString[10], int i);

void fatal(char *str) {
    perror(str);
    exit(1);
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
    int i = 0;
    pid_t pid;
    while (1) {
        fputs(prompt, stdout);
        fgets(cmdline, BUFSIZ, stdin);
        cmdline[strlen(cmdline) - 1] = '\0';
        makelist(cmdline, " \t", cmdvector, MAX_CMD_ARG);

        if (is_builtin() > 0) continue;

        switch (pid = fork()) {
            case 0:
                execvp(cmdvector[0], cmdvector);
                fatal("main()");
            case -1:
                fatal("main()");
            default:
                wait(NULL);
        }
    }
    return 0;
}

int is_builtin() {
    // 내장 명령어인지 확인
    for (int i = 0; i < 3; ++i)
        if (!strcmp(cmdvector[0], commands[i])) {
            // 내장 명령어 수행
            do_command(cmdvector[0], cmdvector, i);
            return 1;
        }

    return 0;
}

void do_command(char *command, char *pString[MAX_CMD_ARG], int command_number) {
    switch (command_number) {
        case CD:
            change_directory();
//            printf("cd\n");
            break;
        case PWD:
            get_current_directory();
//            printf("pwd\n");
            break;
        case EXIT:
            exit(0);
        default:
            fatal("do_command()");
    }
}
