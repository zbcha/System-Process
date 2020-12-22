/*Author: Bochao Zhang B00748967
 *File:   command.c
 *Data:   2020/5/27
 *Note:   use makefile to compile and "./a2" to run
 */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "command.h"

typedef void (*inner_func)(char* cmd);

static int last_status;

static int myisspace(char);
static size_t str_tokenize(char* str, char** tokens, const size_t n);
static size_t cmd_tokenize(char* str, char** tokens, const size_t n, char seperator);
static void do_cmds(char** str, const size_t n, int isBackground);
static int checkbackground(char* str, int length);
static void zombie_reaper (int signal);
static void block_zombie_reaper (int signal);
static int run_cmds(char* commands[], int cmd_size);
static int deal_cmds(char* commands[], int cmd_size, int pipein);
static int deal_cmd(char *command);
static void cd_func(char* cmd);
static void exit_func(char* cmd);

static char* inner_cmd[] = {"cd","exit"};
static inner_func inner_funcs[] = {cd_func, exit_func}; 
static int inner_cmd_size = 2;

void init_command() {
	signal(SIGCHLD, zombie_reaper);
}

/*
 * Typical reaper.
 */
void zombie_reaper (int signal) {
    int ignore;
    while (waitpid(-1, &ignore, WNOHANG) >= 0)
        /* NOP */;
}

/*
 * Dummy reaper, set as signal handler in those cases when we want
 * really to wait for children.  Used by run_it().
 *
 * Note: we do need a function (that does nothing) for all of this, we
 * cannot just use SIG_IGN since this is not guaranteed to work
 * according to the POSIX standard on the matter.
 */
void block_zombie_reaper (int signal) {
    /* NOP */
}

int checkbackground(char* command, int length) {
	int i = length - 1;
	if(command[i] == '&' && i != 0 && myisspace(command[i-1])){
    	command[i] = '\0';
    	return 1;
    }
    return 0;
}

size_t cmd_tokenize(char* str, char** tokens, const size_t n, char seperator) {
  size_t tok_size = 1;
  tokens[0] = str;
  
  size_t i = 0;
  while (i < n) {
    if (str[i] == seperator) {
      str[i] = '\0';
      i++;

	  tokens[tok_size++] = str + i;
    }
    else 
      i++;
  }

  return tok_size;
}

size_t str_tokenize(char* str, char** tokens, const size_t n) {
  size_t tok_size = 1;
  tokens[0] = str;
  
  size_t i = 0;
  while (i < n) {
    if (myisspace(str[i])) {
      str[i] = '\0';
      i++;
      for (; i < n && myisspace(str[i]); i++) 
        /* NOP */;
      if (i < n) {
        tokens[tok_size] = str + i;
        tok_size++;
      }
    }
    else 
      i++;
  }

  return tok_size;
}

int myisspace(char ch){
	return ch == '\r' || ch == '\n' || ch == ' ' || ch == '\t';
}

char* remove_comment(char* command) {
	int i;
	for(i = 0; command[i]; ++i)
		if(command[i] == '#' && (i == 0 || myisspace(command[i-1]))){
			command[i] = '\0';
			break;
		}
	return command;
}

char* trim(char* command, int length){
	int i;
	char* ret;

	for(i = 0; i < length && myisspace(command[i]); ++i);
	ret = command + i;

	if(i == length)
		return ret;

	int end;
	for(end = length - 1; end >= i && myisspace(command[end]); --end);
	command[end + 1] = '\0';

	return ret;
}

void do_command(const char* cmd) {
	if(cmd == NULL || cmd[0] == '\0')
		return;

	int length = strlen(cmd);
	char * command = (char*)malloc(sizeof(char)*(length+1));
	strcpy(command, cmd);

	// trim space
	char* newcmd = trim(command, length);
	newcmd = remove_comment(newcmd);
	length  = strlen(newcmd);
	if(length == 0){
		free(command);
		return;
	}

	length = strlen(newcmd);

	// split command by ';'
	char* cmds[256]; // buffer for the tokenized commands
	int cmd_size = cmd_tokenize(newcmd, cmds, length, ';');

	// if there are more than 1 command, call "do_command" recusiouly to deal with each command
	if(cmd_size > 1) {
		int i;
		for(i = 0; i < cmd_size; ++i)
			do_command(cmds[i]);
		free(command);
		return;
	}

	// otherwise we only need to handle one command, here is the process to handle one command
    int isBackground = 0;
    // if the last no space character is & and it's not the first word, it's a background command
    if(checkbackground(command, length)){
    	isBackground = 1;
    }

    // split command by |
    cmd_size = cmd_tokenize(newcmd, cmds, length, '|');

    do_cmds(cmds, cmd_size, isBackground);
    free(command);
}

void do_cmds(char** cmds, const size_t n, int bg){
	size_t i;

	for(i = 0; i < n; ++i){
		int len = strlen(cmds[i]);
		cmds[i] = trim(cmds[i], len);
		len = strlen(cmds[i]);
		if(len == 0 || checkbackground(cmds[i], len)){
			fprintf(stderr, "myshell: syntax error near unexpected token `|`\n");
			return;
		}
	}

	if (bg) {  // background command, we fork a process that
               // awaits for its completion
        int bgp = fork();
        if (bgp == 0) { // child executing the command
            last_status = run_cmds(cmds, n);

            printf("Process %d terminated\n", getpid());
			if(last_status != 0){
				printf("Exit %d\n", WEXITSTATUS(last_status));
			}
            
            exit(0);
        }else if(bgp == -1) {
        	fprintf(stderr, "myshell: fork a child process to execute command failed.\n");
        }
    }
    else {  // foreground command, we execute it and wait for completion
    	if( n == 1 ){
    		char *cmd = cmds[0];
    		for(i = 0; i < inner_cmd_size; ++i){
    			if(cmd == strstr(cmd, inner_cmd[i])){
    				inner_funcs[i](cmd);
    				return;
    			}
    		}
    	}

        last_status = run_cmds(cmds, n);
    }
}

int deal_cmd(char *command){
	// fprintf(stderr, "command:%s, pid:%d, ppid:%d\n", command, getpid(), getppid());
	char* tokens[256];
	int i, j;

	for(i = strlen(command) - 1; i >= 0; --i) {
		if(command[i] == '>'){
			for(j = i + 1; command[j] && myisspace(command[j]); ++j);
			if(command[j] == '\0'){
				fprintf(stderr, "-myshell: syntax error near unexpected token 'newline'\n");
				close(STDIN_FILENO);
				return 2;
			}

			int start = j;
			while(command[j] && !myisspace(command[j]))
				++j;
			int end = j;

			char filename[128];
			strncpy(filename, command + start, end - start);
			filename[end - start] = '\0';

			int fout = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			if(fout == -1){
				fprintf(stderr, "-myshell: create file '%s' failed.\n", filename);
				close(STDIN_FILENO);
				return 2;
			}
			
			dup2(fout, STDOUT_FILENO);
			close(fout);

			start = i;
			for(j = 0; command[j + end]; ++j)
				command[start + j] = command[j + end];
			command[start + j] = '\0';
		}
	}

	char* second_cmd = NULL;
	for(i = 0; command[i]; ++i) {
		if (command[i] == '&' && i != 0 && command[i-1] == '&') {
			command[i-1] = '\0';
			second_cmd = command + i + 1;
			break;
		}
	}


	int token_size = str_tokenize(command, tokens, strlen(command));
	tokens[token_size] = NULL;

	if(second_cmd == NULL) {
		execvp(tokens[0], tokens);
		close(STDIN_FILENO);
		fprintf(stderr, "myshell: %s: command not found\n", command);
	} else { // there are two command seperated by "&&"
		int bgp = fork();
        if (bgp == 0) { // child executing the command
            execvp(tokens[0], tokens);
            close(STDIN_FILENO);
			fprintf(stderr, "myshell: %s: command not found\n", command);
            exit(errno);
        }else if(bgp == -1) {
        	fprintf(stderr, "myshell: fork a child process to execute command failed.\n");
        } else {
        	int status;
        	int ret = waitpid(-1, &status, 0);
        	if(status == 0) {
        		second_cmd = trim(second_cmd, strlen(second_cmd));
				int token_size = str_tokenize(second_cmd, tokens, strlen(second_cmd));
				tokens[token_size] = NULL;
        		execvp(tokens[0], tokens);
	            close(STDIN_FILENO);
				fprintf(stderr, "myshell: %s: command not found\n", second_cmd);
        	}
        }
	}
    return errno;
}

int deal_cmds(char* commands[], int cmd_size, int pipein){
	// fprintf(stderr, "commands[0] = %s, cmd_size = %d\n", commands[0], cmd_size);
	if(cmd_size == 1){
		int childp = fork();
	    if (childp == -1) {
	    	fprintf(stderr, "myshell: fork a child process to execute command failed.\n");
	    	return -1;
	    }else if (childp == 0) {
	    	if(pipein != -1){
				dup2(pipein, STDIN_FILENO);
				close(pipein);
			}
	    	exit(deal_cmd(commands[0]));
	    } else {
	    	int ret = 0;
	    	int status;
	    	while(ret != -1){
	    		ret = waitpid(-1, &status, 0);
	    	}

			return status;
	    }
	}

	int fd[2];

	int ret;
	if( (ret = pipe(fd)) != 0 ) {
		perror("myshell: create a pipe failed: ");
		return ret;
	}

	int childp = fork();
    if (childp == -1) {
    	fprintf(stderr, "myshell: fork a child process to execute command failed.\n");
    	return -1;
    }else if (childp == 0) {
    	if(pipein != -1){
			dup2(pipein, STDIN_FILENO);
			close(pipein);
		}
    	if( (ret = dup2(fd[1], STDOUT_FILENO)) == -1 ) {
			perror("myshell: dup2 failed: ");
			return ret;
		}
		close(fd[1]);
    	close(fd[0]);
    	close(pipein);
    	exit(deal_cmd(commands[0]));
    } else {
    	close(fd[1]);
    	ret = deal_cmds(commands + 1, cmd_size - 1, fd[0]);
    	close(fd[0]);
    	return ret;
    }
}

int run_cmds(char* commands[], int cmd_size) {
    // we really want to wait for children so we inhibit the normal
    // handling of SIGCHLD
    signal(SIGCHLD, block_zombie_reaper);

    int status = deal_cmds(commands, cmd_size, -1);

	// we restore the signal handler now that our baby answered
    signal(SIGCHLD, zombie_reaper);

    return status;
}

static void cd_func(char* cmd) {
	char* tokens[128];
	int token_size = str_tokenize(cmd, tokens, strlen(cmd));
	if(token_size == 1)
		last_status = chdir(getpwuid(getuid())->pw_dir);
	else {
		if(tokens[1][0] == '~') {
			char temp[128];
			strcpy(temp, getpwuid(getuid())->pw_dir);
			strcat(temp, temp + 1);
			last_status = chdir(temp);
		}else{
			last_status = chdir(tokens[1]);
		}

		if(last_status == -1){
			fprintf(stderr, "chdir: no such file or directory: %s\n", tokens[1]);
		}
	}
}

static void exit_func(char* cmd) {
	exit(0);
}
