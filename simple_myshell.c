#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "commands/change_directory.h"
#include "commands/get_current_directory.h"

#define MAX_CMD_ARG 10
#define BUF_SIZE 256

#define FOREGROUND 0
#define BACKGROUND 1

#define CD 0
#define PWD 1
#define EXIT 2

const char *prompt = "myshell> ";
char *cmdvector[MAX_CMD_ARG];
char cmdline[BUF_SIZE];

const char *commands[3] = {"cd", "pwd", "exit"};

int is_builtin();

void do_command(char *string, char *cmdlist[10], int i);

void do_process(int type);

void wait_or_not(int type, pid_t i);

int type_check(int count);

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
    int type;
    while (1) {
        fputs(prompt, stdout);
        fgets(cmdline, BUF_SIZE, stdin);
        cmdline[strlen(cmdline) - 1] = '\0';
//        printf("fputs: %d cmdline: %s\n", fputs(cmdline, stdout), cmdline);
        int arguments_count = makelist(cmdline, " \t", cmdvector, MAX_CMD_ARG);

        if(!arguments_count) continue;

        if (is_builtin() > 0) continue;

        type = type_check(arguments_count);
        do_process(type);
        memchr(cmdline, (char*)0, BUF_SIZE);
    }
    return 0;
}

int type_check(int count) {
    char *last_argument = cmdvector[count - 1];
    int last_length = strlen(last_argument);

    if (last_argument[last_length - 1] == '&') {
        if (last_length == 1)
            cmdvector[count - 1] = NULL;
        else if (last_length > 1)
            cmdvector[count - 1][last_length - 1] = '\0';
        else
            fatal("type_check()");

        return BACKGROUND;
    } else
        return FOREGROUND;
}

void do_process(int type) {
    pid_t pid;
    switch (pid = fork()) {
        case 0:
            execvp(cmdvector[0], cmdvector);
            fatal("do_process()");
        case -1:
            fatal("do_process()");
        default:
            wait_or_not(type, pid);
    }
}

void wait_or_not(int type, pid_t pid) {
    if (type == FOREGROUND)
        waitpid(pid,NULL,0);
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

void do_command(char *command, char *cmdlist[MAX_CMD_ARG], int command_number) {
    switch (command_number) {
        case CD:
            if (change_directory(cmdlist[1]) == -1)
                fatal("cd");
            break;
            /**
             * 보통 pwd을 /bin/pwd를 가지고 있어서 구현을 안해도 실행이 되지만
             * 엄밀히 따지면 pwd은 기본적으로 쉘 내장함수라서 일단은 구현을 했습니다.
             * 해당 프로젝트는 시스템콜을 통해 쉘의 내장 함수로서 pwd가 작동됩니다.
             */
        case PWD:
            if (get_current_directory() == -1)
                fatal("pwd");
            break;
        case EXIT:
            exit(0);
        default:
            fatal("do_command()");
            break;
    }
}
