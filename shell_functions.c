#include "shell_functions.h"

void print_prompt(){
    char current_working_directory[1000];
    char user[1000];
		getlogin_r(user, sizeof(user));
		getcwd(current_working_directory, sizeof(current_working_directory));
    printf("%s%s@my-terminal:", KGRN, user);
    printf("%s%s%s $ ",KCYN, current_working_directory, KNRM);
    fflush(stdout);
}

bool is_empty(char* command){
  return command == NULL;
}

void create_child(char** command_arguments){
  restore_terminal_signals();
  pid_t my_pid = getpid();
  new_process_group(my_pid);
  execvp(command_arguments[0], command_arguments);
  fprintf(stderr, "%sError: command not found: %s\n", KRED, command_arguments[0]);
  exit(-1);
}

int wait_foreground_process(pid_t pid_fork){
  pid_t pid_wait;
  int wait_info;
	int wait_status;
	enum status wait_status_result;
  char buffer[2000];
	char termination_type[16];
  waitpid(-1, &wait_status, WUNTRACED | WCONTINUED);

  set_terminal(getpid());

  if (WIFEXITED(wait_status)){
    wait_info = WEXITSTATUS(wait_status);
    sprintf(buffer, "parent: child %d terminated with exit(%d)\n", pid_fork, wait_info);
  }
  if (WIFSIGNALED(wait_status)){
        wait_info = WTERMSIG(wait_status);
        sprintf(buffer, "parent: child %d kill by signal %d\n", pid_fork, wait_info);
      }
    if (WIFSTOPPED(wait_status)){
        wait_info = WSTOPSIG(wait_status);
        sprintf(buffer, "parent: child %d stopped by signal %d\n", pid_fork, wait_info);
      }
    if (WIFCONTINUED(wait_status)){
        sprintf(buffer, "parent: child %d continued\n", pid_fork);
      }
    printf("%s%S\n", KMAG, buffer);
    return wait_info;
}

bool is_built_in_command(char* command){
  bool ans = false;
  if(strcmp(command, "cd") == 0){
    ans = true;
  }else if(strcmp(command, "jobs") == 0){
    ans = true;
  }
  return ans;
}

void execute_built_in_command(char** command_arguments){
  if(strcmp(command_arguments[0], "cd") == 0){
    execute_cd(command_arguments);
  }else if(strcmp(command_arguments[0], "jobs") == 0){
    execute_jobs();
  }
}

void execute_cd(char** command_arguments){
  int there_was_an_error;
  if(is_empty(command_arguments[1])){
    char user[100];
    char buffer[20];
    strcpy(buffer, "/home/");
    getlogin_r(user, sizeof(user));
    strcat(buffer, user);
    there_was_an_error = chdir(buffer);
  }else if(command_arguments[1][0] == '.' || command_arguments[1][0] == '/'){
    there_was_an_error = chdir(command_arguments[1]);
  }else{
    char current_working_directory[1000];
    getcwd(current_working_directory, sizeof(current_working_directory));
    strcat(current_working_directory, "/");
    strcat(current_working_directory, command_arguments[1]);
    there_was_an_error = chdir(current_working_directory);
  }
  if(there_was_an_error){
    fprintf(stderr, "%sError: cd: %s: No such file or directory\n", KRED, command_arguments[1]);
  }
}

void execute_jobs(){
  print_job_list(list_of_jobs);
}

void child_handler(int signal_number){
  int i;
  int wait_status, wait_info;
  for(i = 0; i < list_size(list_of_jobs);i++){
    waitpid(-1, &wait_status, WUNTRACED | WNOHANG);
    if (WIFEXITED(wait_status)){
      wait_info = WEXITSTATUS(wait_status);
      printf("Process exited! %d\n", wait_info);
    }
    if (WIFSTOPPED(wait_status)){
      wait_info = WSTOPSIG(wait_status);
      printf("Process stopped! %d\n", wait_info);
    }
    if (WIFCONTINUED(wait_status)){
      printf("Process continued! %d\n", wait_info);
    }
  }
}


void process_delayed_signals(int signal_number){
  if (signal_number == SIGINT){

  }
}
