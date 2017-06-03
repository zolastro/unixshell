#include "job_control.h"
#include "shell_functions.h"
#include <string.h>


int main(void)
{
	char command_input[MAX_LENGTH];
	bool is_background;
	char *command_arguments[MAX_LENGTH/2];

	pid_t pid_fork;

	list_of_jobs = new_list("List of jobs");

	signal(SIGCHLD, child_handler);

	while (true){
		ignore_terminal_signals();
    print_prompt();
		get_command(command_input, MAX_LENGTH, command_arguments, &is_background);

		if(!is_empty(command_arguments[0])){
			if(is_built_in_command(command_arguments[0])){
				execute_built_in_command(command_arguments);
			}else{
				pid_fork = fork();
				if(pid_fork == 0){
					create_child(command_arguments);
				}else{
					add_job(list_of_jobs, new_job(pid_fork, command_arguments[0], (is_background) ? BACKGROUND : FOREGROUND));
					if(is_background){
						printf("%sLog: process [%d] -> %s is running in is_background.%s\n", KMAG, pid_fork, command_input, KNRM);
					}else{
						int wait_info = wait_foreground_process(pid_fork);
						if(wait_info == -1){
							exit(-1);
						}

						//printf("%sLog: foreground pid: %d, command: %s, info: %d%s\n", KMAG, pid_fork, command_input, wait_info, KNRM);
					}
				}
			}
		}
	}
}
