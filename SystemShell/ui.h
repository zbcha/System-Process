/**
 * @file
 *
 * Text-based UI functionality. These functions are primarily concerned with
 * interacting with the readline library.
 */

#ifndef _UI_H_
#define _UI_H_

char *read_command(void);
void set_is_tty(int);
int is_tty();

#endif
