#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "command.h"
#include "array.h"

#define MAX_CMD_ARG 50
#define BUF_SIZE 256

const char *prompt = "myshell> ";
char *cmdvector[MAX_CMD_ARG];
char cmdline[BUF_SIZE];
int redirect_input = 0;
int redirect_output = 1;
static sigjmp_buf jmpbuf;
static volatile sig_atomic_t canjump;

void split(char *cmdSet[5][MAX_CMD_ARG], char *cmd[MAX_CMD_ARG]);

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
        char *session[5][MAX_CMD_ARG];
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

        /**
         * 한 줄을 입력받고 파이프 기호를 delimeter로 명령어들을 쪼갬
         */
        char **arg = cmdvector;
        int row = 0;
        int col = 0;
        while (*arg != NULL) {
            if (!strcmp(*arg, "|")) {
                session[row++][col] = NULL; // 명령의 마지막 인자로 NULL을 입력
                col = 0;
            } else {
                session[row][col++] = *arg;
            }
            arg++;
        }
        session[row][col] = NULL; // 명령의 마지막 인자로 NULL을 입력

        /**
         * session 배열에서 명령어들을 하나씩 실행
         */
        int p[2];
        for(unsigned i = 0; i< row+1;i++) {
            int count = 0;
            while (session[i][count] != NULL) count++;

            /**
             * 백그라운드 실행인지 체크
             */
            type = type_check(&count, session[i]);

            /**
             * 리다이렉션을 체크
             */
            int input_index = indexOf(session[i], count, "<");
            int output_index = indexOf(session[i], count, ">");

            if(input_index != -1){
                char *input_filename = session[i][input_index + 1];
                if((redirect_input = open(input_filename, O_RDONLY)) == -1){
                    perror(input_filename);
                    exit(1);
                }
                session[i][input_index] = NULL;
                session[i][input_index+1] = NULL;
            }
            if (output_index != -1) {
                char *output_filename = session[i][output_index + 1];
                if((redirect_output = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1){
                    perror(output_filename);
                    exit(1);
                }
                session[i][output_index] = NULL;
                session[i][output_index+1] = NULL;
            }

            pipe(p);
            pid_t pid;

            /**
             * 명령어 실행
             */
            switch (pid = fork()) {
                case 0:
                    // SET SIGNAL
                    signal(SIGINT, SIG_DFL);
                    signal(SIGQUIT, SIG_DFL);
                    signal(SIGSTOP, SIG_DFL);

                    // SET PIPE
                    if (i != row) {
                        dup2(p[1], redirect_output);
                    }
                    close(p[0]);

                    // SET REDIRECT
                    dup2(redirect_input, 0);
                    dup2(redirect_output, 1);

                    execvp(session[i][0], session[i]);
                    perror("do_process()");
                case -1:
                    perror("do_process()");
                default:
                    wait_or_not(type, pid);
                    close(p[1]);
                    redirect_input = p[0];
            }
        }
    }
    return 0;
}

void split(char *cmdSet[5][MAX_CMD_ARG], char *cmd[MAX_CMD_ARG]) {
    char **arg = cmd;

    int row = 0;
    int col = 0;
    while (*arg != NULL) {
        if (!strcmp(*arg, "|")) {
            col = 0;
            row++;
        } else {
            cmdSet[row][col++] = *arg;
        }
        arg++;
    }
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
