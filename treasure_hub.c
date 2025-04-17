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

pid_t monitor_pid = -1;

int filter(const struct dirent *name)
{
  return 1;
}

void start_monitor(){
  if(monitor_pid != -1){
    write(1, "Monitorul merge deja\n", strlen("Monitorul merge deja\n"));
    return;
  }
  if((monitor_pid = fork()) < 0){
    perror(NULL);
    exit(-1);
  }
  

  if(monitor_pid == 0){

    write(1, "Monitorul porneste\n", strlen("Monitorul porneste\n"));
    struct sigaction s;
    s.sa_handler = SIG_IGN;
    s.sa_flags = 0;
    sigaction(SIGUSR1, &s, NULL);
    sigaction(SIGUSR2, &s, NULL);
    sigaction(SIGINT, &s, NULL);
    sigaction(SIGTERM, &s, NULL);

    while(1){
      pause();
    }
  }
}

void list_hunts(){
  
  struct dirent **namelist;
  int n = scandir(".", &namelist, filter, alphasort);
  if (n == -1) {
    perror("scandir");
    exit(EXIT_FAILURE);
  }
  while(n--){
    if(strncmp(namelist[n]->d_name, "game", strlen("game")) == 0){
      write(1, namelist[n]->d_name, strlen(namelist[n]->d_name));
      struct dirent **namelist2;
      int m = scandir( namelist[n]->d_name, &namelist2, filter, alphasort);
      if (m == -1) {
	perror("scandir");
	exit(EXIT_FAILURE);
      }
      int count = 0;
      while(m--){
	if(strncmp(namelist2[n]->d_name, "treasure", strlen("treasure")) == 0){
	  count++;
	}
      }
      char number[20];
      int x = sprintf(number, " %d treasures\n", count);
      write(1, number, x);
    }
  }
 
  
}

void list_treasures(){
  char buff[100];
  write(1, "Introduceti nume hunt:", strlen("Introduceti nume hunt:"));
  read(1, buff, sizeof(buff));
  buff[strlen(buff)-1] = '\0';
  struct dirent **namelist;
  int n = scandir(buff, &namelist, filter, alphasort);
  if (n == -1) {
    perror("scandir");
    exit(EXIT_FAILURE);
  }
  while(n--){
    if(strncmp(namelist[n]->d_name, "treasure", strlen("treasure")) == 0){
      write(1, namelist[n]->d_name, strlen(namelist[n]->d_name));
      write(1, " Size:", strlen(" Size:"));
      memset(buff, 0, sizeof(buff));
      sprintf(buff, "%d", namelist[n]->d_reclen);
      write(1, buff, strlen(buff));
      write(1, " Type:", strlen(" Type:"));
      memset(buff, 0, sizeof(buff));
      if(namelist[n]->d_type == DT_REG)strcpy(buff, "regular file\n");
      else if(namelist[n]->d_type == DT_DIR)strcpy(buff, "directory\n");
      write(1, buff, strlen(buff));
      
    }
  }
  memset(buff, 0, sizeof(buff));
  
}

void view_treasure(){

  char buff[100];
  write(1, "Introduceti nume hunt:", strlen("Introduceti nume hunt:"));
  read(1, buff, sizeof(buff));
  buff[strlen(buff)-1] = '\0';
  struct dirent **namelist;
  int n = scandir(buff, &namelist, filter, alphasort);
  if (n == -1) {
    perror("scandir");
    exit(EXIT_FAILURE);
  }
  write(1, "Introduceti nume treasure:", strlen("Introduceti nume treasure:"));
  memset(buff, 0, sizeof(buff));
  read(1, buff, sizeof(buff));
  buff[strlen(buff)-1] = '\0';
  while(n--){
    if(strncmp(namelist[n]->d_name, buff, strlen(buff)) == 0){
      write(1, namelist[n]->d_name, strlen(namelist[n]->d_name));
      write(1, " Size:", strlen(" Size:"));
      memset(buff, 0, sizeof(buff));
      sprintf(buff, "%d", namelist[n]->d_reclen);
      write(1, buff, strlen(buff));
      write(1, " Type:", strlen(" Type:"));
      memset(buff, 0, sizeof(buff));
      if(namelist[n]->d_type == DT_REG)strcpy(buff, "regular file\n");
      else if(namelist[n]->d_type == DT_DIR)strcpy(buff, "directory\n");
      write(1, buff, strlen(buff));
      
    }
  }
  memset(buff, 0, sizeof(buff));
}

void send_signal(int sig){
  if(monitor_pid == -1){
    write(1, "Monitorizarea nu merge\n", strlen("Monitorizarea nu merge\n"));
    return;
  }

  switch(sig){
  case SIGUSR1:{
    list_hunts();
    break;
  }
  case SIGUSR2:{
    list_treasures();
    break;
  }
  case SIGINT:{
    view_treasure();
    break;
  }
  }
  
}

void stop_monitor(){

  if(monitor_pid == -1){
    write(1, "Monitorul e oprit deja\n", strlen("Monitorul e oprit deja\n"));
    return;
  }

  write(1, "Monitorul se opreste\n", strlen("Monitorul se opreste\n"));
  kill(monitor_pid, SIGTERM);
  monitor_pid = -1;
  usleep(1000000);
}

int main(void){

  char buff[100];

  while(read(0, buff, sizeof(buff))){
    if(strncmp(buff, "start_monitor", strlen("start_monitor")) == 0){
      start_monitor();
    }	
    else if(strncmp(buff, "list_hunts", strlen("list_hunts")) == 0){
      send_signal(SIGUSR1);
    }
    else if(strncmp(buff, "list_treasures", strlen("list_treasures")) == 0){
      send_signal(SIGUSR2);
    }
    else if(strncmp(buff, "view_treasure", strlen("view_treasure")) == 0){
      send_signal(SIGINT);
    }
    else if(strncmp(buff, "stop_monitor", strlen("stop_monitor")) == 0){
      stop_monitor();
    }
    else if(strncmp(buff, "exit", strlen("exit")) == 0){
      if(monitor_pid == -1)break;
      write(1, "Nu se poate opri programul, monitorul inca ruleaza\n", strlen("Nu se poate opri programul, monitorul inca ruleaza\n"));
    }
    else{
      write(1, "Comanda gresita\n", strlen("Comanda gresita\n"));
    }
  }

     
  return 0;
}
