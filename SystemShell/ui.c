#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"

static int isTty;
void set_is_tty(int val){
    isTty = val;
}

int is_tty(){
    return isTty;
}

char command_line[1024];
char *read_command(void)
{

    size_t len;
    char *line = NULL;
    printf("$ ");
    fflush(stdout);
    if(getline(&line, &len, stdin) == -1)
        exit(0);

    if(line){
        strcpy(command_line, line);
        free(line);
    }else{
        exit(0);
    }

    return command_line;
}
