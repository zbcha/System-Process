#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command.h"
#include "ui.h"

int main(void)
{
    set_is_tty(isatty(STDIN_FILENO));

    init_command();

    while (true) {
        char *command = read_command();
        if (command == NULL) {
            break;
        }

        do_command(command);
    }

    return 0;
}
