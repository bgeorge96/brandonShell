#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define v "bgeorgeSHv1"
#define PATH "PATH"
#define SIZE 20

void handler(int sig) {
  while (waitpid(-1, NULL, 0) > 0) {}
}

void myCd(char *path){
  if(chdir(path) != 0){
    printf("%s\n", strerror(errno));
  }
}

void createArgvList(char *inStr, char **outList,char *delimiter){
  for (size_t i = 0; i < SIZE; i++) {
    outList[i] = NULL;
  }

  char *cur_str = strtok(inStr, delimiter);
  for (size_t i = 0; cur_str != NULL; i++) {
    outList[i] = cur_str;
    cur_str = strtok(NULL, " ");
  }
}

void runAtPath(char *path_with_executable){
  extern char **environ;
  char *argv_list[SIZE];
  char **ptr_argv = argv_list;
  char *paths[SIZE];
  char *pathEnv = malloc(sizeof(char)*100);
  strcpy(pathEnv, getenv(PATH));

  int idx = 0;
  char *curStr = malloc(sizeof(char)*100);
  curStr = strtok(pathEnv, ":");

  // adding the dot to the beginning of the paths
  paths[idx] = ".\0";
  idx++;

  while (curStr != NULL) {
    paths[idx] = curStr;
    curStr = strtok(NULL, ":");
    if (curStr != NULL) {
      idx++;
    }
  }


  free(curStr);
  free(pathEnv);

  createArgvList(path_with_executable, ptr_argv, " ");

  if (path_with_executable[0] == '.' || path_with_executable[0] == '/') {
    execve(argv_list[0], argv_list, environ);
  } else {
    char str_command[100];
    strcpy(str_command, argv_list[0]);
    char *tmp = malloc(sizeof(char)*100);
    while (idx >= 0) {

      strcpy(tmp, paths[idx]);
      strcat(tmp, "/");
      strcat(tmp, str_command);
      argv_list[0] = tmp;

      if(execve(argv_list[0], argv_list, environ) == 0 ){
        return;
      }
      idx = idx - 1;
    }
    free(tmp);
  }


}

void removeSpecialChars(int needle, char *haystack) {
  char *needleInHay = strchr(haystack, needle);
  if (needleInHay != NULL) {
    *needleInHay = '\0';
  }
}

void redirection(int charToFind, char *stringToSearch, char *path) {
  char *output_char = strchr( stringToSearch, charToFind);
  if (output_char != NULL) {
    char *tmp = strchr(output_char, ' ') +1;
    char here[100];
    strcpy(here, path);
    strcat(here, "/");
    strcat(here, tmp);

    if (charToFind == '>') {
      close(1);
    } else if (charToFind == '<') {
      close(0);
    }
    open(here, O_CREAT|O_RDWR);
  }
}

int main(int argc, char **argv, char **envp) {
  pid_t pid;
  char untouched_command[100];
  char totalpath[100];
  char command[100];
  char *cur_str;

  while (1) {
    signal(SIGCHLD, handler);

    getcwd(totalpath, 100);
    printf("%s$ ", totalpath);
    scanf ("%[^\n]%*c", untouched_command);

    memcpy(command, untouched_command, 100);

    cur_str = strtok(command, " ");

    if (strcmp("pwd", cur_str) == 0) {
      char path[100];
      printf("%s\n", getcwd(path, 100));
    }
    else if (strcmp("exit", cur_str) == 0) {
      exit(0);
    }
    else if (strcmp("cd", cur_str) == 0) {
      char *path = strtok(NULL, " ");
      myCd(path);
    }
    else if (strcmp("version", cur_str) == 0) {
      printf("%s\n", v);
    }
    else {
      pid = fork();
      if (pid == 0) {
        memcpy(command, untouched_command, 100);
        char tear_up[100];
        memcpy(tear_up, untouched_command, 100);

        // checks to see if there is an redirection
        redirection('>', tear_up, totalpath);
        redirection('<', tear_up, totalpath);

        // checks to see if there are special chars and nulls them
        removeSpecialChars('&', command);
        removeSpecialChars('>', command);
        removeSpecialChars('<', command);

        runAtPath(command);
        printf("%s\n", strerror(errno));
        exit(0);
      } else if(pid > 0){
        if (strchr(untouched_command, '&') == NULL) {
          waitpid(0, NULL, 0);
        }
      }
    }
  }
  return 0;
}
