#ifndef _SHELL_FUNCTIONS_H
#define _SHELL_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <string.h>


#include "job_control.h"

typedef int bool;
#define true 1
#define false 0

#define MAX_LENGTH 256

//Colors
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

job_list list_of_jobs;


void print_prompt();
bool is_empty(char* command);
void create_child(char** command_arguments);
int wait_foreground_process(pid_t pid_fork);
bool is_built_in_command(char* command);
void execute_built_in_command(char** command_arguments);
void execute_cd(char** command_arguments);
void execute_jobs();
void child_handler(int signal_number);


#endif
