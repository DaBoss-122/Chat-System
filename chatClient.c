#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>        // for exit
#include <netinet/in.h>    // for sockaddr_in struct
#include <strings.h>       // for bzero
#include <arpa/inet.h>     // for inet_pton
#include <unistd.h>        // for close
#include <sys/select.h>    // for select and related structures
#include <string.h>        // for strerror
#include <errno.h>         // for errno
#include <fcntl.h>         // for fcntl
#include <string.h>       // for strcmp

#define ECHO_PORT 9000
#define MAXLINE 4096
#define JOIN 0
#define LEAVE 1
#define TALK 2
#define LIST 3
#define DIRECT 4
#define ERROR 5

extern int errno;
void do_error(const char *text, int enr);
int main(int argc, char **argv) {
    int servsockfd, sockfd, numbers, clientsocklen;
    int res,flags;
    fd_set set, rset;
    char message[MAXLINE+1];
    char recline[MAXLINE+1];
    char *username;
    int ipaddr;
    short port=9000;
    struct sockaddr_in servaddr, clientaddr;
    int amountReceived;
    sockfd = socket(AF_INET, SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (argc<3) {
        printf("Not enough arguments. Include the ip name.\n");
        return 0;
    }
    else {
        inet_pton(AF_INET,argv[1],&(servaddr.sin_addr));
        //printf("%d\n",port);
        username = argv[2];
    }
    int userLen=strlen(username);
   connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

      message[0] = JOIN;
      message[1] = (userLen>>8)&0x00FF;
      message[2] = userLen&0x00FF;
      strcpy(&(message[3]),username);
      send(sockfd,message,userLen+3,0);
      res = recv(sockfd,recline,MAXLINE,0);
      if (res<=0) {
        printf("Received not a response.\n");
      } else {
        if (recline[0]==0) {
          printf("Name Accepted\n");
          //break;
        }
        else {
          printf("User name already taken.\n");
          return 0;
        }
   }
   printf("%s>\n",username);
  flags = fcntl(sockfd, F_GETFL, 0);

   if (flags == -1)
      do_error("fcntl(F_GETFD) error", errno);

   flags |= O_NONBLOCK;

   res = fcntl(sockfd, F_SETFL, flags);

   if(res == -1)
      do_error("fcntl(F_SETFD) error", errno);

   flags = fcntl(0, F_GETFL, 0);

   if (flags == -1)
      do_error("fcntl(F_GETFD) error", errno);

   flags |= O_NONBLOCK;

   res = fcntl(0, F_SETFL, flags);

   if(res == -1)
      do_error("fcntl(F_SETFD) error", errno);


   // zero select sets for select
   FD_ZERO(&set);
   FD_ZERO(&rset);
  // infinite loop to process activity
   for(;;)
   {
//Trying to receive non-blocking another way. Ignore this mess.
     if (( res = read(0, recline, MAXLINE))>0) {
//printf("Got it\n");
       if (strcmp(recline,"@exit")==0) {
          message[0] = LEAVE;
          send(sockfd,message,1,0);
      } else {
          message[0] = TALK;
          message[1] = (res>>8)&0x00FF;
          message[2] = res&0x00FF;
          strcpy(&(message[3]),recline);
          send(sockfd,message,res+3,0);
          printf("%s>",username);
      }
     }
     res = recv(sockfd, recline, MAXLINE, 0);
     if (res == -1)
      {
       continue;
      }
     else if (res > 0)
     {
      recline[res] = 0;
      printf("%s",&(recline[3]));
     }
  }
   //close(sockfd);
   printf("Program Terminated Successfully.\n");
   return 0;

}
void do_error(const char *text, int enr){
   fprintf(stderr,"%s %d: %s\n", text, errno, strerror(errno));
   exit(errno);
}

