#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>

int filter(const struct dirent *name)
{
  return 1;
}

int main(void){
  struct dirent **namelist;
  int n = scandir(".", &namelist, filter, alphasort);
  if (n == -1) {
    perror("scandir");
    exit(EXIT_FAILURE);
  }
  while(n--){
    
    if(strncmp(namelist[n]->d_name, "game", strlen("game")) == 0){
      
      struct dirent **namelist2;
      int m = scandir( namelist[n]->d_name, &namelist2, filter, alphasort);
      if (m == -1) {
	perror("scandir2");
	exit(EXIT_FAILURE);
      }
      int count[100] = {0};
      char names[100][100], nume[100], value[10];
      int index = 0;
      chdir(namelist[n]->d_name);
      while(m--){
	if(strncmp(namelist2[m]->d_name, "treasure", strlen("treasure")) == 0){
	  
	  int pipefd[2];

	  if(pipe(pipefd) == -1){

	    perror(NULL);
	    exit(-1);
	  }
	  pid_t x;
	  if((x = fork()) < 0){
	    perror(NULL);
	    exit(-1);
	  }
	  if(x == 0){
	    dup2(pipefd[1], 1);
	    execl("/bin/sed", "sed", "-n", "2p",namelist2[m]->d_name , NULL);
	    printf("eroare\n");
	  }
	  wait(&x);
	  read(pipefd[0], nume, sizeof(nume));
	  
	  pid_t x2;
	  if((x2 = fork()) < 0){
	    perror(NULL);
	    exit(-1);
	  }
	  if(x2 == 0){
	    dup2(pipefd[1], 1);
	    execl("/bin/sed", "sed", "-n", "5p",namelist2[m]->d_name , NULL);
	    printf("eroare\n");
	  }
	  wait(&x2);
	  read(pipefd[0], value, sizeof(value));
	  
	  int v = atoi(value);
	  int ok = 0;
	  for(int i = 0; i < index; i++){
	    if(strcmp(nume, names[i]) == 0){
	      count[i] += v;
	      ok = 1;
	    }
	  }
	  if(ok == 0){
	    strcpy(names[index], nume);
	    count[index] = v;
	    index++;
	  }
	}
	//free(namelist2[m]);
      }
      //free(namelist2);
      chdir("..");
      for(int i = 0; i < index; i++){
	printf("%s%d\n", names[i], count[i]);
      }
      //free(namelist[n]);
    }
    //free(namelist);
  }

  return 0;
}
