#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>

void make_treasure(){

  write(1, "Insert treasure number:", strlen("Insert treasure number:"));
  
  char buf[10];
  
  read(0, buf, sizeof(buf));
  buf[strlen(buf)-1] = '\0';
  
  char nume[30] = "treasure";
  strcat(nume, buf);
  strcat(nume, ".txt");
  
  int fd = open(nume, O_CREAT|O_WRONLY, 0777);
  
  buf[strlen(buf)] = '\n';
  buf[strlen(buf)+1] = '\0';
  write(fd, buf, strlen(buf));
  
  write(1, "Insert treasure user:", strlen("Insert treasure user:"));
  read(0, buf, sizeof(buf));
  write(fd, buf, strlen(buf));
  
  char buff[100];
  
  write(1, "Insert treasure gps:", strlen("Insert treasure gps:"));
  read(0, buff, sizeof(buff));
  write(fd, buff, strlen(buff));
  memset(buff, 0, sizeof(buff));
  
  write(1, "Insert treasure clue:", strlen("Insert treasure clue:"));
  read(0, buff, sizeof(buff));
  write(fd, buff, strlen(buff));
  memset(buff, 0, sizeof(buff));
  
  write(1, "Insert treasure value:", strlen("Insert treasure value:"));
  read(0, buff, sizeof(buff));
  write(fd, buff, strlen(buff));

  close(fd);

  fd = open("logged_hunt.txt", O_CREAT|O_WRONLY|O_APPEND, 0777);
  write(fd, "add ", sizeof("add "));
  write(fd, nume, strlen(nume));
  write(fd, "\n", 1);
  close(fd);
}

void add(char *game){
  mkdir(game, 0777);
  chdir(game);
  make_treasure();
  chdir("..");
}

int filter(const struct dirent *name)
{
  return 1;
}

void list(char *game){
  write(1, game, strlen(game));
  write(1, "\n", 1);
  struct stat st;
  lstat(game, &st);
  char buff[100];
  sprintf(buff, "%ld\n", st.st_size);
  write(1, buff, strlen(buff));
  memset(buff, 0, sizeof(buff));
  strftime(buff, sizeof(buff), "%D %T\n", gmtime(&st.st_mtim.tv_sec));
  write(1, buff, strlen(buff));
  struct dirent **namelist;
  int n = scandir(game, &namelist, filter, alphasort);
  if (n == -1) {
    perror("scandir");
    exit(EXIT_FAILURE);
  }
  while(n--){
    if(strcmp(namelist[n]->d_name, ".") == 0 || strcmp(namelist[n]->d_name, "..") == 0)continue;
    printf("%s\n", namelist[n]->d_name);
  }
  chdir(game);
  int fd = open("logged_hunt.txt", O_CREAT|O_WRONLY|O_APPEND, 0777);
  write(fd, "list\n", sizeof("list\n"));
  chdir("..");
}

void view(char *game, char *treasure){
  chdir(game);
  strcat(treasure, ".txt");
  struct stat st;
  lstat(treasure, &st);
  char buff[100];
  sprintf(buff, "%ld\n", st.st_size);
  write(1, buff, strlen(buff));
  memset(buff, 0, sizeof(buff));
  strftime(buff, sizeof(buff), "%D %T\n", gmtime(&st.st_mtim.tv_sec));
  write(1, buff, strlen(buff));
  int fd = open("logged_hunt.txt", O_CREAT|O_WRONLY|O_APPEND, 0777);
  write(fd, "view ", sizeof("view "));
  write(fd, treasure, strlen(treasure));
  write(fd, "\n", 1);
  chdir("..");
}

void rm_hnt(char *game){
  struct dirent **namelist;
  int n = scandir(game, &namelist, filter, alphasort);
  if (n == -1) {
    perror("scandir");
    exit(EXIT_FAILURE);
  }
  chdir(game);
  while(n--){
    if(strcmp(namelist[n]->d_name, ".") == 0 || strcmp(namelist[n]->d_name, "..") == 0)continue;
    remove(namelist[n]->d_name);
  }
  chdir("..");
  rmdir(game);
}

void rm_tr(char *game, char *treasure){

  chdir(game);
  strcat(treasure, ".txt");
  remove(treasure);
  int fd = open("logged_hunt.txt", O_CREAT|O_WRONLY|O_APPEND, 0777);
  write(fd, "remove ", sizeof("remove "));
  write(fd, treasure, strlen(treasure));
  write(fd, "\n", 1);
  chdir("..");
}

int main(int argc, char **argv){

  if(argc < 2){
    perror(NULL);
    exit(-1);
  }

  switch(argv[1][0]){
  case 'a':{

    add(argv[2]);
    break;
  }
  case 'l':{
    list(argv[2]);
    break;
  }
  case 'v':{
    view(argv[2], argv[3]);
    break;
  }
  case 'r':{
    if(argc == 4)rm_tr(argv[2], argv[3]);
    else rm_hnt(argv[2]);
    break;
  }
  }
  char name[20] = "logged_hunt";
  char path[20] ;
  strcpy(path, argv[2]);
  strcat(path, "/logged_hunt.txt");
  strcat(name, argv[2]+strlen(argv[2])-1);
  strcat(name, ".txt");
  if(symlink(path, name) != 0){
    if(errno != EEXIST){
      perror("symlink() error");
      unlink(name);
    }
    else if(argv[1][0] == 'r' && argc != 4)unlink(name);
  }
  
  return 0;
}
