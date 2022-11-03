#ifndef __COMMAND_H_
#define __COMMAND_H_

#include <sys/types.h>

#define MAX_CMD_ARG 10

int is_builtin(char *commands[MAX_CMD_ARG]);

void do_command(char *commands[10], int i);

void do_process(int type, char *commands[MAX_CMD_ARG]);

void wait_or_not(int type, pid_t i);

int type_check(int count, char* commands[MAX_CMD_ARG]);

#endif
