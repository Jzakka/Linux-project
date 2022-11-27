#ifndef __COMMAND_H_
#define __COMMAND_H_

#include <sys/types.h>

#define MAX_CMD_ARG 10

int is_builtin(char *commands[MAX_CMD_ARG]);

void do_command(char *commands[10], int i);

void redirect_resolve(char *commands[MAX_CMD_ARG], int count, int* redirect_input, int* redirect_output);

void run_line(char *session[5][MAX_CMD_ARG], int row, int *redirect_input, int* redirect_output);

void do_process(int type, char *commands[MAX_CMD_ARG], int i, int p[],  int row,
                int* redirect_input, int* redirect_output);

void wait_or_not(int type, pid_t i);

int type_check(int* count, char* commands[MAX_CMD_ARG]);

#endif
