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
volatile sig_atomic_t done = 0;

int filter(const struct dirent *name)
{
  return 1;
}

void handler_done(){
  done = 1;
}

void handle_sigchld(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
    write(1, "Monitorul s-a terminat\n", 
          strlen("Monitorul s-a terminat\n"));
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

  kill(getppid(), SIGTERM);
 
  
}

void list_treasures(){
  char buff[100];
  write(1, "Introduceti nume hunt:", strlen("Introduceti nume hunt:"));
  read(0, buff, sizeof(buff));
  buff[strlen(buff)-1] = '\0';
  pid_t x;
  if((x = fork()) < 0){
    perror(NULL);
    exit(-1);
  }
  if(x == 0){
    execl("/usr/bin/gcc", "/usr/bin/gcc", "-Wall", "-o", "bin2", "treasure_hunter.c", NULL);
    printf("eroare\n");
  }
  wait(NULL);
  if((x = fork()) < 0){
    perror(NULL);
    exit(-1);
  }
  if(x == 0){
    execl("./bin2", "./bin2", "list", buff,  NULL);
    printf("eroare\n");
   }
  wait(NULL);
  kill(getppid(), SIGTERM);
  
}

void view_treasure(){

  char buff[100], buff1[100];
  write(1, "Introduceti nume hunt:", strlen("Introduceti nume hunt:"));
  read(0, buff, sizeof(buff));
  buff[strlen(buff)-1] = '\0';
  
  write(1, "Introduceti nume treasure:", strlen("Introduceti nume treasure:"));
  memset(buff1, 0, sizeof(buff));
  read(0, buff1, sizeof(buff));
  buff1[strlen(buff1)-1] = '\0';

  pid_t x;
  if((x = fork()) < 0){
    perror(NULL);
    exit(-1);
  }
  if(x == 0){
    execl("/usr/bin/gcc", "/usr/bin/gcc", "-Wall", "-o", "bin2", "treasure_hunter.c", NULL);
    printf("eroare\n");
  }
  wait(&x);
  if((x = fork()) < 0){
    perror(NULL);
    exit(-1);
  }
  if(x == 0){
    execl("./bin2", "./bin2", "view", buff, buff1,  NULL);
    printf("eroare\n");
   }
  wait(&x);
  kill(getppid(), SIGTERM);
  }

void send_signal(int sig){
  if(monitor_pid == -1){
    write(1, "Monitorizarea nu merge\n", strlen("Monitorizarea nu merge\n"));
    return;
  }

  kill(monitor_pid, sig);
  /*switch(sig){
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
  }*/
  
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
    s.sa_handler = list_hunts;
    s.sa_flags = 0;
    sigaction(SIGUSR1, &s, NULL);
    s.sa_handler = list_treasures;
    sigaction(SIGUSR2, &s, NULL);
    s.sa_handler = view_treasure;
    sigaction(SIGINT, &s, NULL);
    s.sa_handler = handle_sigchld;
    sigaction(SIGTERM, &s, NULL);

    while(1){
      pause();
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
  usleep(5000000);
}

int main(void){


  struct sigaction s_done;
  s_done.sa_handler = handler_done;
  s_done.sa_flags = 0;
  sigemptyset(&s_done.sa_mask);
  sigaction(SIGTERM, &s_done, NULL);
  
  char buff[100];

  while(read(0, buff, sizeof(buff))){
    if(strncmp(buff, "start_monitor", strlen("start_monitor")) == 0){
      start_monitor();
    }	
    else if(strncmp(buff, "list_hunts", strlen("list_hunts")) == 0){
      done = 0;
      send_signal(SIGUSR1);
      while(!done)pause();
    }
    else if(strncmp(buff, "list_treasures", strlen("list_treasures")) == 0){
      done = 0;
      send_signal(SIGUSR2);
      while(!done)pause();
    }
    else if(strncmp(buff, "view_treasure", strlen("view_treasure")) == 0){
      done = 0;
      send_signal(SIGINT);
      while(!done)pause();
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
