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
#include <string.h>       // for strcmp
#include <time.h>

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
  struct userConnection* cur = firstConnection;
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
struct userConnection* query(char[] name, int len) {
  struct userConnection* cur = firstConnection;
  while (cur->length!=len&&(strcmp(cur->username,name))) {
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
   struct userConnection* cur = firstConnection;
   struct userConnection* prev = NULL;

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
   FILE *fp;
   char fileText[MAXLINE + 1];
   fp = fopen("cteam_chatserver.txt", "w");
   if(fp == NULL){
      printf("file can't be opened\n");
      exit(1);
   }

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
               printf("Client connection attempt\n");
               bzero(&caddr, sizeof(caddr));
               csock = accept(ssock, (struct sockaddr *) &caddr, &csize);
               if (csock == -1)
                  do_error("accept() error", errno);
                  fprintf(fp, "Refused connection from %s on port %d\n", inet_ntoa((struct in_addr)caddr.sin_addr), ntohs(caddr.sin_port));
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
				      printf("Received: %d %s\n",res, &(recline[3]));
					   //recline[res] = 0;
                  //printList();
                  //Switch statement for operations
                  switch(recline[0]){
                     case JOIN:
                        printf("Joining.\n");
                        //Extract User Name
                        unameLen = res-2;
                        //`printf("Before\n");

                        //printf("After");
                        //1. iterate through list of usernames and find the filedescriptor
                        found = findUserName(&(recline[3]),unameLen);

                        printf("User found is %d\n",found);
                        if (found<0){
                           //2.0 Set username & length in userConnection
                           found = findUserFD(i);
                           printf("User file descriptor is: %d\n",found);
                           if (found<=0&&unameLen>1) {

                              fprintf(fp, "The username %s was accepted. Joining chat\n", &(recline[3]))

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
                              //printf("DBG i: %d found: %d",i,found);
                              recline[0]+=1;
                              send(i,&(recline[0]),1,0);
                              fprintf(fp, "The username %s was rejected - name in use\n", &(recline[3]))
                           }
                                    //insert(i, &recline[3], );
                        }
                        //2.1 or if username is used, send used to client
                        else {
                           recline[0]+=1;
                           send(i, &(recline[0]), 1, 0);
                           fprintf(fp, "The username %s was rejected - name in use\n", &(recline[3]))
                        }
                           break;
                     // End of JOIN

                     case LEAVE:
                        currentConn = userRemove(i);
                        strcpy(&(message[5]),currentConn->username);
                        strcpy(&(message[currentConn->length+4])," is leaving the chat**\n");
                        fprintf(fp, "User %s disconnected\n", &(message[currentConn->length+4]))
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
                        //heawder 3 username is len message is res
                        mesLen = currentConn->length+res;
                        //printf("mess len %x",mesLen);
                        message[0] = TALK;
                        message[2] = (char)(mesLen&0x00FF);
                        message[1] = (char)(mesLen>>8);
                        message[3] = '[';
                        //message[4] = ' ';
                        //printf("low bye %x high bye %x",message[1],message[2]);
                        //2. get the message (in recline)
                        strcpy(&(message[4]),currentConn->username);
                        // printf(message);
                        message[currentConn->length+3] = ']';
                        message[currentConn->length+4] = ' ';
                        //strcpy(&(message[currentConn->length+2]),": ");
                        // printf(message);

                        strcpy(&(message[currentConn->length+5]),&(recline[3]));
                        //printf(&(message[3]));
                        //3. concatonate username first (Username: message)
                        //Length currentConn->length+2+res-1
                        sendToAll(message,mesLen+3);
                        fprintf(fp, message)
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




                        /*if(res == -1)
                        {
                           // send error - clear socket from list

                     }*/
	               }
                  //printList();
	            }
	         }
	      }
	   }
	}
   fclose(fp);
   return 0;
}
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
      //printf("Comparing: %s and %s\n",cur->username,name);
      if (0==strcmp(cur->username,name)) {
         return cur->fd;
      }
   }
   cur=cur->next;
   }
   //printf("Not found\n");
   return -1;
}

int findUserFD(int fd){
	struct userConnection *cur = firstConnection;
   struct userConnection *prev = NULL;

   if(firstConnection == NULL) {
      return -1;
   }
	//Loop through the list of
	while(cur->fd != fd) {
		if(cur->next == NULL) {
			//User wasn't found
			return 0;
		}
		else {
			prev = cur;
			cur = cur->next;
		}
	}
	//Found the user if we got here
	return 1;
}
int sendToAll(char *message, int len) {
   struct userConnection *cur = firstConnection;
   if (firstConnection == NULL) {
      return -1;
   }
   while (cur!=NULL) {
      send(cur->fd,message,len,0);
      printf("Sent %s to %d\n",&(message[3]),cur->fd);
      cur = cur->next;
   }
   return 1;
}
//Inserts a user - assumes that length includes the null pointer in its calculation
void insert(int fd, char *username, int length) {
   struct userConnection *node = (struct userConnection*) malloc(sizeof(struct userConnection));

   node->fd = fd;
   //Assumes that length includes the null pointer char!
   node->username = malloc(sizeof(char) * (length));
   node->length = length;
   node->next = firstConnection;
   firstConnection = node;
   strcpy(node->username,username);

}
void printList() {
   printf("List: ");
   struct userConnection *cur = firstConnection;
   if(firstConnection==NULL) {
      printf("NULL\n");
      return;
   }
   while(cur!=NULL) {

		//prev = cur;
      printf("%d, ",cur->fd);
		cur = cur->next;

	}
}

void do_error(const char *text, int enr) {
   fprintf(stderr,"%s %d: %s\n", text, errno, strerror(errno));
   exit(errno);
}
