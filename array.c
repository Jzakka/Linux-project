#include <string.h>

int indexOf(char** stringArray, int arraySize, char* target){
    for(unsigned i = 0; i< arraySize;i++){
        if(!strcmp(stringArray[i], target)){
            return i;
        }
    }
    return -1;
}