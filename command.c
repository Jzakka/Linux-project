#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "array.h"

#include "command.h"
#include "builtin/change_directory.h"
#include "builtin/get_current_directory.h"

#define FOREGROUND 0
#define BACKGROUND 1
#define MAX_CMD_ARG 10

#define CD 0
#define PWD 1
#define EXIT 2

const char *builtin_commands[3] = {"cd", "pwd", "exit"};

void printE(char *str) {
    perror(str);
    exit(1);
}

int type_check(int* count, char* commands[MAX_CMD_ARG]) {
    char *last_argument = commands[*count - 1];
    int last_length = strlen(last_argument);

    if (last_argument[last_length - 1] == '&') {
        if (last_length == 1)
            commands[--(*count)] = NULL;
        else if (last_length > 1)
            commands[--(*count)][last_length - 1] = '\0';
        else
            printE("type_check()");

        return BACKGROUND;
    } else
        return FOREGROUND;
}

void redirect_resolve(char *commands[MAX_CMD_ARG], int count, int* redirect_input, int* redirect_output){
    int input_index = indexOf(commands, count, "<");
    int output_index = indexOf(commands, count, ">");

    if (input_index != -1) {
        char *input_filename = commands[input_index + 1];
        if ((*redirect_input = open(input_filename, O_RDONLY)) == -1) {
            perror(input_filename);
            exit(1);
        }
        commands[input_index] = NULL;
        commands[input_index + 1] = NULL;
    }
    if (output_index != -1) {
        char *output_filename = commands[output_index + 1];
        if ((*redirect_output = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
            perror(output_filename);
            exit(1);
        }
        commands[output_index] = NULL;
        commands[output_index + 1] = NULL;
    }
}

void do_process(int type, char *commands[MAX_CMD_ARG], int i, int p[],  int row,
                int* redirect_input, int* redirect_output) {
    pipe(p);
    pid_t pid;

    switch (pid = fork()) {
        case 0:
            // SET SIGNAL
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGSTOP, SIG_DFL);

            // SET PIPE
            if (i != row) {
                dup2(p[1], *redirect_output);
            }
            close(p[0]);

            // SET REDIRECT
            dup2(*redirect_input, 0);
            dup2(*redirect_output, 1);

            execvp(commands[0], commands);
            perror("do_process()");
        case -1:
            perror("do_process()");
        default:
            wait_or_not(type, pid);
            close(p[1]);
            *redirect_input = p[0];
    }
}

void wait_or_not(int type, pid_t pid) {
    if (type == FOREGROUND){
        waitpid(pid,NULL,0);
//        printf("pid: %d\n", pid);
    }
}

int is_builtin(char *commands[MAX_CMD_ARG]) {
    // 내장 명령어인지 확인
    for (int i = 0; i < 3; ++i)
        if (!strcmp(commands[0], builtin_commands[i])) {
            // 내장 명령어 수행
            do_command(commands, i);
            return 1;
        }

    return 0;
}

void do_command(char *commands[MAX_CMD_ARG], int command_number) {
    switch (command_number) {
        case CD:
            if (change_directory(commands[1]) == -1)
                perror("cd");
            break;
            /**
             * 보통 pwd을 /bin/pwd를 가지고 있어서 구현을 안해도 실행이 되지만
             * 엄밀히 따지면 pwd은 기본적으로 쉘 내장함수라서 일단은 구현을 했습니다.
             * 해당 프로젝트는 시스템콜을 통해 쉘의 내장 함수로서 pwd가 작동됩니다.
             */
        case PWD:
            if (get_current_directory() == -1)
                perror("pwd");
            break;
        case EXIT:
            exit(0);
        default:
            printE("do_command()");
            break;
    }
}

