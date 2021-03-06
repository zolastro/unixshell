#include "shell_functions.h"

void print_prompt(){
    char current_working_directory[1000];
    char user[1000];
		getlogin_r(user, sizeof(user));
		getcwd(current_working_directory, sizeof(current_working_directory));
    printf("%s%s@my-terminal:%s", KGRN, user,KNRM);
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
  fprintf(stderr, "%sError: command not found: %s%s\n", KRED, command_arguments[0],KNRM);
  exit(-1);
}

int wait_foreground_process(pid_t pid_fork){
  pid_t wait_status;
  int wait_info;
	enum status wait_status_result;
  job* current_job = get_item_bypid(list_of_jobs, pid_fork);


  block_SIGCHLD();
  set_terminal(pid_fork);
  killpg(pid_fork, SIGCONT);

  waitpid(pid_fork, &wait_status, WUNTRACED);
  set_terminal(getpid());

  if (WIFEXITED(wait_status)){
    wait_info = WEXITSTATUS(wait_status);
    printf("%sparent: child %d terminated with exit(%d)%s\n", KMAG,pid_fork, wait_info, KNRM);
    delete_job(list_of_jobs, current_job);
  }
  if (WIFSIGNALED(wait_status)){
        wait_info = WTERMSIG(wait_status);
        printf("%sparent: child %d kill by signal %d%s\n",KMAG, pid_fork, wait_info, KNRM);
        delete_job(list_of_jobs, current_job);
      }
    if (WIFSTOPPED(wait_status)){
        wait_info = WSTOPSIG(wait_status);
        current_job->state = STOPPED;
        printf("%sparent: child %d stopped by signal %d%s\n",KMAG, pid_fork, wait_info, KNRM);
      }
    if (WIFCONTINUED(wait_status)){
        printf("%sparent: child %d continued%s\n",KMAG, pid_fork, KNRM);
        current_job->state = FOREGROUND;
    }
    unblock_SIGCHLD();
    return wait_info;
}

bool is_built_in_command(char* command){
  bool ans = false;
  if(strcmp(command, "cd") == 0){
    ans = true;
  }else if(strcmp(command, "jobs") == 0){
    ans = true;
  }else if(strcmp(command, "fg") == 0){
    ans = true;
  }
  return ans;
}

void execute_built_in_command(char** command_arguments){
  if(strcmp(command_arguments[0], "cd") == 0){
    execute_cd(command_arguments);
  }else if(strcmp(command_arguments[0], "jobs") == 0){
    execute_jobs();
  }else if(strcmp(command_arguments[0], "fg") == 0){
    execute_fg(command_arguments);
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

void execute_fg(char** command_arguments){
  int job_number = 1;
  job* current_job;
  if(!is_empty(command_arguments[1])){
    job_number = atoi(command_arguments[1]);
  }
  if(list_size(list_of_jobs) < 1){
    printf("%sThere isn't any process running in background%s\n", KMAG, KNRM);
    return;
  }else if(job_number < 1 ||list_size(list_of_jobs) < job_number){
      printf("%sbash: fg: %d: no such job%s\n", KRED, job_number, KNRM);
      return;
  }
  current_job = get_item_bypos(list_of_jobs, job_number);
  wait_foreground_process(current_job->pgid);
}

void child_handler(int signal_number){
  int wait_status;
  job* current_job;
  int i;
  for(i = 1; i <= list_size(list_of_jobs);i++){
    current_job = get_item_bypos(list_of_jobs, i);
    waitpid(current_job->pgid, &wait_status, WUNTRACED | WNOHANG);
    if (WIFEXITED(wait_status)){
      delete_job(list_of_jobs, current_job);
    }
    if (WIFSTOPPED(wait_status)){
      current_job->state = STOPPED;
    }
    if (WIFCONTINUED(wait_status)){
      current_job->state = FOREGROUND;
    }
  }
}
