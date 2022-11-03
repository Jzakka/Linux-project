#include <unistd.h>
#include <string.h>
#include <pwd.h>

int change_directory(char *path) {
    if(path == NULL || !strcmp(path, "~"))
        path = getpwuid(getuid())->pw_dir;

    return chdir(path);
}