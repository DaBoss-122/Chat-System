// Chat Server - C Team

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
#include <signal.h>

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
//Inserts a user - assumes that length includes the null pointer in its calculation
void insert(int fd, char *username, int length);
struct userConnection* userRemove(int fd);
struct userConnection* getUser(int fd);
int sendToAll(char *message, int len);
int findUserFD(int fd);
int findUserName(char *name, int len);
void printList();
FILE *fp;
//Functions:
//Sends to all the sockets

//Check if a username is set

//Check if username is used already


struct userConnection {
	int fd;
	char *username;
	int length;
	struct userConnection* next;
};

struct userConnection *firstConnection;

void sighad(int signo) {
   fclose(fp);
   exit(0);
}

int main(int argc, char **argv)
{
   char recline[MAXLINE + 1];
   signal(SIGINT,sighad);
   signal(SIGTERM,sighad);
   struct sockaddr_in servaddr, caddr;
   //FILE *fp;
   char fileText[MAXLINE + 1];
   fp = fopen("cteam_chatserver.txt", "a");
   if(fp == NULL){
      printf("file can't be opened\n");
      exit(1);
   }

   int ssock;  // server sockfd
   int csock;  // client sockfd when accepting connction
   int tsock;  // temp sockfd for working with select

   int flags;  // for fcntl calls
   int res;    // result for function calls
   socklen_t csize;

   int nrrec = 0;
   fd_set set, rset;
   char taken = '0';
   char message[MAXLINE+1];

   int found = 0; //Was the username found or not
   struct userConnection *currentConn = NULL;
   short mesLen = 0;
   int unameLen=0;
   ssock = socket(AF_INET, SOCK_STREAM, 0);
   const int       optVal = 1;
   const socklen_t optLen = sizeof(optVal);
   setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, optLen);
   if (ssock < 0)
      do_error("socket() error", errno);

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port = htons(ECHO_PORT);

   res = bind(ssock, (struct sockaddr*) &servaddr, sizeof(servaddr));
   if (res == -1)
      do_error("bind() error", errno);

   flags = fcntl(ssock, F_GETFL, 0);

   if (flags == -1)
      do_error("fcntl(F_GETFD) error", errno);

   flags |= O_NONBLOCK;

   res = fcntl(ssock, F_SETFL, flags);

   if(res == -1)
      do_error("fcntl(F_SETFD) error", errno);

   listen(ssock, 5);

   // zero select sets for select
   FD_ZERO(&set);
   FD_ZERO(&rset);

   // set server socket on main select set
   FD_SET(ssock, &set);

	printf("Server running.\n");
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
              printf("Client connection attempt\n");
              bzero(&caddr, sizeof(caddr));
               csock = accept(ssock, (struct sockaddr *) &caddr, &csize);
               if (csock == -1) {
                  do_error("accept() error", errno);
                  fprintf(fp, "Refused connection from %s on port %d\n", inet_ntoa((struct in_addr)caddr.sin_addr), ntohs(caddr.sin_port));
               }
               else
               {
                  // add new client socket to master set
                  FD_SET(csock, &set);
				      //Add to userConnection
                  printf("New Client Connected %d",csock);
                  fprintf(fp, "Accepted connection from %s on port %d\n", inet_ntoa((struct in_addr)caddr.sin_addr), ntohs(caddr.sin_port));

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
                        printf("Joining.\n");
                        //Extract User Name
                        unameLen = res-2;

                        //1. iterate through list of usernames and find the filedescriptor
                        found = findUserName(&(recline[3]),unameLen);

                        if (found<0){
                           //2.0 Set username & length in userConnection
                           found = findUserFD(i);

                           if (found<=0&&unameLen>1) {
                              fprintf(fp, "The username %s was accepted. Joining chat\n", &(recline[3]));
                              insert(i,&(recline[3]),unameLen);


                              send(i,&(recline[0]),1,0);

                              strcpy(&(message[5]),&(recline[3]));
                              strcpy(&(message[res+2])," is joining the chat**\n");
                              mesLen = res+22;
                              message[0] = TALK;
                              message[2] = (char)(mesLen&0x00FF);
                              message[1] = (char)(mesLen>>8);
                              message[3] = '*';
                              message[4] = '*';
                              sendToAll(message,mesLen+3);

                           }
                           else {
                              //Be mean and do not let them change the name.
                              recline[0]+=1;
                              send(i,&(recline[0]),1,0);
                              fprintf(fp, "The username %s was rejected - name in use\n", &(recline[3]));
                           }
                        }
                        //2.1 or if username is used, send used to client
                        else {
                           recline[0]+=1;
                           send(i, &(recline[0]), 1, 0);
                           fprintf(fp, "The username %s was rejected - name in use\n", &(recline[3]));
                        }
                        break;
                     // End of JOIN

                     case LEAVE:
                        currentConn = userRemove(i);
                        strcpy(&(message[5]),currentConn->username);
                        strcpy(&(message[currentConn->length+4])," is leaving the chat**\n");
                        mesLen = currentConn->length+24;
                        message[0] = TALK;
                        message[2] = (char)(mesLen&0x00FF);
                        message[1] = (char)(mesLen>>8);
                        message[3] = '*';
                        message[4] = '*';
                        sendToAll(message,mesLen+3);

                        close(i);
                        FD_CLR(i, &set);
                        // remove from userConnection
                        //currentConn = userRemove(i);
                        free(currentConn->username);
                        free(currentConn);
                        //DO not free again.
                        currentConn = NULL;
                        break;
                     // End of LEAVE

                     case TALK:
                        //1. check to see if username is set/get usernamea
                        currentConn = getUser(i);
                        if (currentConn==NULL) {
                           send(i,&(recline[0]),1,0);
                           printf("Not on server");
                           break;
                        }

                        //header 3 username is len message is res
                        mesLen = currentConn->length+res;

                        message[0] = TALK;
                        message[2] = (char)(mesLen&0x00FF);
                        message[1] = (char)(mesLen>>8);
                        message[3] = '[';

                        //2. get the message (in recline)
                        strcpy(&(message[4]),currentConn->username);
                        message[currentConn->length+3] = ']';
                        message[currentConn->length+4] = ' ';

                        strcpy(&(message[currentConn->length+5]),&(recline[3]));
                        //3. concatonate username first (Username: message)
                        sendToAll(message,mesLen+3);
                        //4. call method to send to everyone
                        break;
                     // End of TALK

                     case LIST:
                        //blah blah blah
                        break;
                     case DIRECT:
                        //blah blah blah
                        break;
                     case ERROR:
                        //blah blah blah
                        break;

	               }
	            }
	         }
	      }
	   }
	}
   fclose(fp);
   return 0;
}
// End of main

//Return -1 if username not found else return the fd of the owner.
int findUserName(char *name,int len) {
   //printf("Finding user name\n");
   struct userConnection *cur = firstConnection;
   if (firstConnection == NULL) {
      //printf("List Empty\n");
      return -1;
   }
   //printf("Starting Loop\n");
   while (cur!=NULL) {

   if (cur->length == len) {
      if (0==strcmp(cur->username,name)) {
         return cur->fd;
      }
   }
   cur=cur->next;
   }
   return -1;
}

int findUserFD(int fd){
	struct userConnection *cur = firstConnection;
   struct userConnection *prev = NULL;

   if(firstConnection == NULL) {
	   printf("firstConnection is invalid.\n");
      return -1;
   }
	//Loop through the list of
	while(cur->fd != fd) {
		if(cur->next == NULL) {
		   //User wasn't found
			printf("User FD not found.\n");
			return 0;
		}
		else {
			prev = cur;
			cur = cur->next;
		}
	}
	//Found the user if we got here
	printf("User FD is %i\n", fd);
	return 1;
}

int sendToAll(char *message, int len) {
  struct userConnection *cur = firstConnection;
  fprintf (fp, &(message[3]));
   if (firstConnection == NULL) {
      return -1;
   }
   while (cur!=NULL) {
      send(cur->fd,message,len,0);
      cur = cur->next;
   }
   return 1;
}

//Inserts a user - assumes that length includes the null pointer in its calculation
void insert(int fd, char *username, int length) {
   struct userConnection *node;
   // *node = <malloc> doesn't allocate the space but rather assigns node to hold that pointer,
   //    which produces a core dump when we try to assign an actual value to node.
   node = (struct userConnection*) malloc(sizeof(struct userConnection));

   printf("In insert function.\n");
   node->fd = fd;
   //Assumes that length includes the null pointer char!
   node->username = malloc(sizeof(char) * (length));
   printf("Got username var allocated.\n");
   node->length = length;
   printf("Length allocated.\n");
   node->next = firstConnection;
   printf("Stored the 'next' var for the current linked list element");
   firstConnection = node;
   strcpy(node->username,username);

}

struct userConnection *getUser(int fd) {
   struct userConnection *cur = firstConnection;

   if(firstConnection == NULL) {
      return NULL;
   }
   while(cur->fd != fd) {
      if(cur->next == NULL) {
         return NULL;
      } else {
         cur = cur->next;
      }
   }
   return cur;
}

//Removes a user from the list of users
struct userConnection *userRemove(int fd) {
   struct userConnection *cur = firstConnection;
   struct userConnection *prev = NULL;

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

void do_error(const char *text, int enr){
   fprintf(stderr,"%s %d: %s\n", text, errno, strerror(errno));
   exit(errno);
}
