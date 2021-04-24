// techosels.c
// TCP echo server using select

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

#define ECHO_PORT 7
#define MAXLINE 4096
#define JOIN 0
#define LEAVE 1
#define TALK 2
#define LIST 3
#define DIRECT 4
#define ERROR 5

extern int errno;

void do_error(const char *text, int enr);

//Methods:
//Sends to all the sockets

//Check if a username is set

//Check if username is used already


struct userConnection{
	int fd;
	char[] username;
	int length;
	struct userConnection* next;
};

struct userConnection* firstConnection;

void insert(int fd, char[] username, int length) {
   struct userConnection *node = (struct userConnection*) malloc(sizeof(struct userConnection));

   node->fd = fd;
   node->username = username;
   node->length = length;
   node->next = firstConnection;
   firstConnection = node;
}
struct userConnection* query(int fd) {
  struct node* cur = firstConnection;
  while (cur->fd != fd) {
    if (cur->next==NULL) {
    return NULL;
  }
  else {
    cur=cur->next;
  }
  }
  return cur;
}
struct userConnection* userRemove(int fd) {
   struct node* cur = firstConnection;
   struct node* prev = NULL;

   if(firstConnection == NULL) {
      return NULL;
   }
   while(cur->fd != fd) {
      if(cur->next == NULL) {
         return NULL;
      } else {
         prev = cur;
         cur = cur->next;
      }
   }
   if(cur == firstConnection) {
      firstConnection = firstConnection->next;
   } else {
      prev->next = cur->next;
   }
   return cur;
}

int main(int argc, char **argv)
{
   char recline[MAXLINE + 1];

   struct sockaddr_in servaddr, caddr;

   int ssock;  // server sockfd
   int csock;  // client sockfd when accepting connction
   int tsock;  // temp sockfd for working with select

   int flags;  // for fcntl calls
   int res;    // result for function calls
   socklen_t csize;

   int nrrec = 0;
   fd_set set, rset;

   ssock = socket(AF_INET, SOCK_STREAM, 0);

   if (ssock < 0)
      do_error("socket() error", errno);

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port = htons(ECHO_PORT);

   res = bind(ssock, (struct sockaddr*) &servaddr, sizeof(servaddr));
   if (res == -1)
      do_error("bind() error", errno);

   flags = fcntl(ssock, F_GETFD, 0);

   if (flags == -1)
      do_error("fcntl(F_GETFD) error", errno);

   flags |= O_NONBLOCK;

   res = fcntl(ssock, F_SETFD, flags);

   if(res == -1)
      do_error("fcntl(F_SETFD) error", errno);

   listen(ssock, 5);

   // zero select sets for select
   FD_ZERO(&set);
   FD_ZERO(&rset);

   // set server socket on main select set
   FD_SET(ssock, &set);

   // infinite loop to process activity
   for(;;)
   {
      // copy main set to receive set
      rset = set;

      // block indefinitely until someone is ready
      res = select(FD_SETSIZE, &rset, NULL, NULL, NULL);

      if(res == -1)
         do_error("select() error", errno);

      for(int i = 0; i < FD_SETSIZE; i++)
      {
         if(FD_ISSET(i, &rset))
         {
            if(i == ssock) // server is ready to accept
            {
               bzero(&caddr, sizeof(caddr));
               csock = accept(ssock, (struct sockaddr *) &caddr, &csize);
               if (csock == -1)
                  do_error("accept() error", errno);
               else
               {
                  // add new client socket to master set
                  FD_SET(csock, &set);
				  //Add to userConnection

               }
            }
            else // it's a client socket
            {
               res = recv(i, recline, MAXLINE, 0);
               if (res == -1)
               {
                  // error - clear socket from scan list and close
                  close(i);
                  FD_CLR(i, &set);
               }
               else if (res > 0)
               {
					recline[res] = 0;
					//Switch statement for operations
					switch(recline[0]){
						case JOIN:
							//1. itterate through list of usernames
							//2. find FD i
							//3.1 set username & length in userConnection
							//3.2 or if username is used, send used to client
							break;
						case LEAVE:
							close(i);
							FD_CLR(i, &set);
							// remove from userConnection

							break;
						case TALK:
							//1. check to see if username is set/get username
							//2. get the message (in recline)
							//3. concatonate username first (Username: message)
							//4. call method to send to everyone
							break;
						case LIST:
							//blah blah blah
							break;
						case DIRECT:
							//blah blah blah
							break;
						case ERROR:
							//blah blah blah
							break;




                  if(res == -1)
                  {
                     // send error - clear socket from list

                  }
               }
            }
         }
      }
   }

   return 0;
}

void do_error(const char *text, int enr)
{
   fprintf(stderr,"%s %d: %s\n", text, errno, strerror(errno));
   exit(errno);
}
