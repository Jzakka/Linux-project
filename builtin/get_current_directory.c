#include <stdio.h>
#include <unistd.h>

#define PWD_SIZE 1024

int get_current_directory(){
    char pwd[PWD_SIZE];
    if(getcwd(pwd, PWD_SIZE) == NULL)
        return -1;
    printf("%s\n", pwd);

    return 1;
}