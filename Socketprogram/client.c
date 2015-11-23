#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "linkedlist.h"
#include "utils.h"
#define BACKLOG 10 // how many pending connections queue will hold

	
	
	

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
if (sa->sa_family == AF_INET) {
return &(((struct sockaddr_in*)sa)->sin_addr);
}
return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*set of Global variables*/
char clientIP[INET6_ADDRSTRLEN];
struct node** serverAlist = NULL;
struct node** serverBlist = NULL;
struct node** serverClist = NULL;
struct node** serverDlist = NULL;
int countAB =0;
int countAC =0;
int countAD =0;
int countBC =0;
int countBD =0;
int countCD =0;
int flagforA = 0;
int flagforB = 0;
int flagforC = 0;
int flagforD = 0;
int forthiscaseB = 0;
int forthiscaseC = 0;
char adjacancymatrixA[4][4];
char MSTmatrix[20];

int mCost = 0;
char UDPMessagetoserversA[1024];
char UDPMessagetoserversB[1024];
char UDPMessagetoserversC[1024];
char UDPMessagetoserversD[1024];

static int receivedinfofromserverA = 0;
static int receivedinfofromserverB = 0;
static int receivedinfofromserverC = 0;
static int receivedinfofromserverD = 0;

char globalmessagebuf[1024];
int spanningtreeA[100];
int spanningtreeB[100];
int spanningtreeC[100];
int spanningtreeD[100];
int temp[4][4];
char stringmessage[100];
void printList(struct node *node)
{
 //printf("inside printlist\n");
 
  while (node != NULL)
  {
	 printf("%s is neighbour\n",node->neighbour);
     printf(" %d is cost\n", node->cost);
     node = node->next;
  }
}

/*This linked list code is taken from geeksforgeeks and modified*/
/* Given a reference (pointer to pointer) to the head
   of a list and an int, appends a new node at the end  */
   
 
void append(struct node** head_ref,  char new_data[100] , int cost)
{

    
    struct node* new_node = (struct node*) malloc(sizeof(struct node));
 
    struct node *last = *head_ref; 
 
    
    strcpy(new_node->neighbour,new_data);
    new_node->cost = cost;

    
    new_node->next = NULL;
 
   
    if (*head_ref == NULL)
    {
       *head_ref = new_node;
     
       return;
    }
 

    while (last->next != NULL)
        last = last->next;
 
   
    last->next = new_node;
    
   
    return;
}

/*Formats a message  for sending to  servers via UDP*/
void UDPformatting()

{

	node *tempA = serverAlist;
	node *tempB = serverBlist;
	node *tempC = serverClist;
	node *tempD = serverDlist;
	//char cost [1024];
	//int len;
	char money[1024],neighbour[1024];
	
	strcat(UDPMessagetoserversA,"A");
	while(tempA != NULL)
	{
	strcat(UDPMessagetoserversA,"*");
	strcpy(neighbour,tempA->neighbour);
	strcat(UDPMessagetoserversA,neighbour);
	strcat(UDPMessagetoserversA,"*");
	sprintf(money,"%d",tempA->cost);
	strcat(UDPMessagetoserversA,money);
	tempA = tempA->next;
    }
  
    strcat(UDPMessagetoserversA,"*");
    strcat(UDPMessagetoserversB,"B");
   while(tempB != NULL)
	{
	strcat(UDPMessagetoserversB,"*");
	strcpy(neighbour,tempB->neighbour);
	strcat(UDPMessagetoserversB,neighbour);
	strcat(UDPMessagetoserversB,"*");
	sprintf(money,"%d",tempB->cost);
	strcat(UDPMessagetoserversB,money);
	//strcat(UDPMessagetoservers,"*");
	
	tempB = tempB->next;
    }
   // strcat(UDPMessagetoservers,"ENDB");
   strcat(UDPMessagetoserversB,"*");
    strcat(UDPMessagetoserversC,"C");
    while(tempC != NULL)
	{
	strcat(UDPMessagetoserversC,"*");
	strcpy(neighbour,tempC->neighbour);
	strcat(UDPMessagetoserversC,neighbour);
	strcat(UDPMessagetoserversC,"*");
	sprintf(money,"%d",tempC->cost);
	strcat(UDPMessagetoserversC,money);
	//strcat(UDPMessagetoservers,"*");
	
	tempC = tempC->next;
    }
   // strcat(UDPMessagetoservers,"ENDC");
   strcat(UDPMessagetoserversD,"*");
    strcat(UDPMessagetoserversD,"D");
    while(tempD != NULL)
	{
	strcat(UDPMessagetoserversD,"*");
	strcpy(neighbour,tempD->neighbour);
	strcat(UDPMessagetoserversD,neighbour);
	strcat(UDPMessagetoserversD,"*");
    sprintf(money,"%d",tempD->cost);
	strcat(UDPMessagetoserversD,money);
	//strcat(UDPMessagetoservers,"*");
	tempD = tempD->next;
    }
	//strcat(UDPMessagetoservers,"ENDD");
	 
	
}

/*Function that prints intermediate message*/
void printmessage(char data[MAXDATASIZE])
{
	char local[100],temp[20],localbuff[20];
	strcpy(local,data);
	char *token;
	
	//printf("%s this is received\n",local);
	token=strtok(local,"*");
	
	if(strcmp(token,"A") == 0)
	{
	spanningtreeA[0] = 0;	
	while(token!=NULL)
	{   
	  	token=strtok(NULL,"*");
	  	
	  	if(token!=NULL)
	  	{	
	  	strcpy(temp,token);
	    }
	  	if(strcmp(temp,"serverB")==0)
	  
	  	{
				  adjacancymatrixA[0][1] = 1;			  
				 // printf("\nAB\t");
				  strcat(globalmessagebuf,"AB");
				  strcat(globalmessagebuf,"\t");
				 
				  countAB++;
		} 
		else if(strcmp(temp,"serverC")==0)
	  	{
				//printf("%s is token in A\n",token);
				 
			      //printf("\nAC\t"); 
			      strcat(globalmessagebuf,"AC");
			      strcat(globalmessagebuf,"\t");
			      countAC++;
			     
				adjacancymatrixA[0][2] = 1;
		} 
		else if(strcmp(temp,"serverD")==0)
	  	{
				//printf("%s is token in A\n",token);
				
				// printf("%s is serverAneighbours[1] \n",serverAneighbours[2]); 
				adjacancymatrixA[0][3] = 1;		
				//printf("\nAD\t");
				strcat(globalmessagebuf,"AD");
				strcat(globalmessagebuf,"\t");
			
				countAD++;
	    }
	    else
	    { 
		  //printf("%s is token in A\n",token);	
		  if(token!=NULL)
		  {	
		  //printf("%s\n",token);
		  strcat(globalmessagebuf,token);
		  strcat(globalmessagebuf,"\n");
		  
	     }
	      
		} 	  	  	
	}
		
    }
    else if(strcmp(token,"B") == 0)
    {
		spanningtreeB[1] = 0;
	while(token!=NULL)
	{  
		
	  	token=strtok(NULL,"*");
	  	if(token!=NULL)	
	  	strcpy(temp,token); 	
	  	if(strcmp(temp,"serverA") ==0)
	  	{
			adjacancymatrixA[1][0] = 1;
			if(countAB == 0)
			{	
			//printf("\nBA\t");
			strcat(globalmessagebuf,"BA");
			strcat(globalmessagebuf,"\t");
			flagforB =1;
			countAB++;
		   }
		  
	   	   //printf("%s is token B\n",token);
	    }
	    else if(strcmp(temp,"serverC") ==0)
	  	{
			  adjacancymatrixA[1][2] = 1;
			   //printf("\nBC\t");
			   strcat(globalmessagebuf,"BC");
			   strcat(globalmessagebuf,"\t");
			   countBC++;
			   forthiscaseB =1;	
			 
	  	   //printf("%s is token B\n",token);
	    }
	    else if(strcmp(temp,"serverD") ==0)
	  	{
			adjacancymatrixA[1][3] = 1;
			   //	printf("\nBD\t");
			   	strcat(globalmessagebuf,"BD");
			   	strcat(globalmessagebuf,"\t");
			   	countBD++;
			   	forthiscaseB =1;
			   	
	  	  //printf("%s is token B\n",token);
	    }
	    else
	    { 
			if((token!=NULL && flagforB) || (token!=NULL && forthiscaseB == 1))
		   {
			// printf("%s\n",token);
			flagforB = 0;
			forthiscaseB = 0;
		   strcat(globalmessagebuf,token);
		   strcat(globalmessagebuf,"\n");
		   
	      }
		
		} 		  	
	}	
		
	}
	else if(strcmp(token,"C") == 0)
    {
	while(token!=NULL)
	{   
	  	token=strtok(NULL,"*");
	  	if(token!=NULL)	
	  	strcpy(temp,token); 
	  	if(strcmp(temp,"serverA") == 0)
	  	{	  	
		 adjacancymatrixA[2][0] = 1;
			if(countAC==0)
			{
			//printf("\nCA\t");
			strcat(globalmessagebuf,"CA");
			strcat(globalmessagebuf,"\t");
			countAC++;
			flagforC = 1;
		    } 
	  	//printf("%s is token C\n",token);
	    }
	    else if(strcmp(temp,"serverB") == 0)
	  	{	  	
		 adjacancymatrixA[2][1] = 1;
		  	if(countBC==0)
		  	{
			//printf("\nCB\t");
			strcat(globalmessagebuf,"CB");
			strcat(globalmessagebuf,"\t");
			countBC++;
			flagforC = 1;
		   }
	  //	printf("%s is token C\n",token);
	    }
	    else if(strcmp(temp,"serverD") == 0)
	  	{	  	
		 adjacancymatrixA[2][3] = 1;
		  	//printf("\nCD\t");
		  	strcat(globalmessagebuf,"CD");
		  	strcat(globalmessagebuf,"\t");
		  	countCD++;
		  	forthiscaseC =1;
	  //	printf("%s is token C\n",token);
	    }
	    else
	    {
		   if((token!=NULL && flagforC) || (token!=NULL && forthiscaseC) )
		  // printf("this is culprit %s\n",token);
		  {
			 forthiscaseC = 0;
			 flagforC = 0;
		  strcat(globalmessagebuf,token);
		  strcat(globalmessagebuf,"\n");
	     }
		   	
		}	  	
	}	
		
	}
	
	else if(strcmp(token,"D") == 0)
    {
	while(token!=NULL)
	{   
	  	token=strtok(NULL,"*");
	  	if(token!=NULL)	
	  	strcpy(temp,token); 
	  	if(strcmp(temp,"serverA") == 0)
	  	{	  	
		 adjacancymatrixA[3][0] = 1;
		  if(countAD==0)
		  {
		  //printf("\nDA\t");
		  strcat(globalmessagebuf,"DA");
		  strcat(globalmessagebuf,"\t");
		  countAD++;	
		  flagforD = 1;
	     }
	  	//printf("%s is token D\n",token);
	    }
	    else if(strcmp(temp,"serverB") == 0)
	  	{	  	
		 adjacancymatrixA[3][1] = 1;
		 if(countBD==0)
		 {
		  //printf("\nDB\t");
		  strcat(globalmessagebuf,"DB");
		  strcat(globalmessagebuf,"\t");
		  flagforD = 1;
		  countBD++;
	   }
		 	
	  	//printf("%s is token D\n",token);
	    }
	    else if(strcmp(temp,"serverC") == 0)
	  	{	  	
		 adjacancymatrixA[3][2] = 1;
		  if(countCD==0)
		  {
		  //printf("\nDC\t");
		  strcat(globalmessagebuf,"DC");
		  strcat(globalmessagebuf,"\t");
		  countCD++;
		  flagforD = 1;
	     }
	  	//printf("%s is token D\n",token);
	    }
	    else
	    {
		 if(token!=NULL && flagforD)
		 {
		 //printf("this is culprit %s\n",token);
		 flagforD = 0;
		strcat(globalmessagebuf,token);
		strcat(globalmessagebuf,"\n");
	   }
		}	  	
	  	//printf("%s is token D\n",token);	  	
	}	
		
	}
	
}

/*Sends UDP messages to ServerA*/
int  ServerAUDPconnection()
{
int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
//char *msg;
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_DGRAM;
if ((rv = getaddrinfo("localhost", SERVERAUDPPORT, &hints, &servinfo)) != 0) {
fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
return 1;
}
// loop through all the results and make a socket
for(p = servinfo; p != NULL; p = p->ai_next) {
if ((sockfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) == -1) {
perror("talker client UDP: socket");
continue;
}
break;
}
struct sockaddr_in localAddress;

socklen_t addressLength;
addressLength = sizeof(struct sockaddr_in);


if (p == NULL) {
fprintf(stderr, "talker client UDP: failed to create socket\n");
return 2;
}


//UDPformatting();

if ((numbytes = sendto(sockfd, UDPMessagetoserversA, strlen(UDPMessagetoserversA), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}

printmessage(UDPMessagetoserversA);
//printf("%sDEBUGUdpmessA in A\n",UDPMessagetoserversA);
if ((numbytes = sendto(sockfd, UDPMessagetoserversB, strlen(UDPMessagetoserversB), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
//printf("%sDEBUGUdpmessB in A\n",UDPMessagetoserversB);
printmessage(UDPMessagetoserversB);
if ((numbytes = sendto(sockfd, UDPMessagetoserversC, strlen(UDPMessagetoserversC), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
printmessage(UDPMessagetoserversC);

//printf("msg from server C in A%s\n",UDPMessagetoserversC);
if ((numbytes = sendto(sockfd, UDPMessagetoserversD, strlen(UDPMessagetoserversD), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
printmessage(UDPMessagetoserversD);


//printf("%sDEBUGUdpmessD in A\n",UDPMessagetoserversD);
getsockname(sockfd, (struct sockaddr*)&localAddress,&addressLength);
printf("The Client has sent the network topology to the  Server A with UDP port %s number and %s IP address\n",SERVERAUDPPORT,clientIP);
printf("For this connection with Server A, The Client has UDP port  number %d and IP address %s\n",(int) ntohs(localAddress.sin_port),clientIP);
printf("Server A has this information\n");
printf("Edge====cost\n");
printf("%s",globalmessagebuf);
freeaddrinfo(servinfo);
//printf("talker client UDP: sent %d bytes to %s\n", numbytes,"localhost");
close(sockfd);
return 0;
	
}

/*Sends UDP messages to ServerB*/
int  ServerBUDPconnection()
{
int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
//char *msg;
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_DGRAM;
if ((rv = getaddrinfo("localhost", SERVERBUDPPORT, &hints, &servinfo)) != 0) {
fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
return 1;
}
// loop through all the results and make a socket
for(p = servinfo; p != NULL; p = p->ai_next) {
if ((sockfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) == -1) {
perror("talker client UDP: socket");
continue;
}
break;
}
if (p == NULL) {
fprintf(stderr, "talker client UDP: failed to create socket\n");
return 2;
}
struct sockaddr_in localAddress;
socklen_t addressLength;
addressLength = sizeof(localAddress);


//UDPformatting();

if ((numbytes = sendto(sockfd, UDPMessagetoserversA, strlen(UDPMessagetoserversA), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
//printmessage(UDPMessagetoserversA);
if ((numbytes = sendto(sockfd, UDPMessagetoserversB, strlen(UDPMessagetoserversB), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
//printmessage(UDPMessagetoserversB);
if ((numbytes = sendto(sockfd, UDPMessagetoserversC, strlen(UDPMessagetoserversC), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
//printmessage(UDPMessagetoserversC);
if ((numbytes = sendto(sockfd, UDPMessagetoserversD, strlen(UDPMessagetoserversD), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}

if(((getsockname(sockfd, (struct sockaddr*)&localAddress,&addressLength)) == -1))
{
	perror("getsockname() failed\n");
}
printf("The Client has sent the network topology  to the Server B with UDP port %s number and %s IP address\n",SERVERBUDPPORT,clientIP);
printf("For this connection with Server B, The Client has UDP port number %d and IP address %s\n", ntohs(localAddress.sin_port),clientIP);
printf("Server B has this information\n");
printf("Edge====cost\n");

//printmessage(UDPMessagetoserversD);
printf("%s",globalmessagebuf);
freeaddrinfo(servinfo);
//printf("talker client UDP: sent %d bytes to %s\n", numbytes,"localhost");
close(sockfd);
return 0;
	
}

/*Sends UDP messages to ServerC*/
int ServerCUDPconnection()
{
int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
//char *msg;
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_DGRAM;
if ((rv = getaddrinfo("localhost", SERVERCUDPPORT, &hints, &servinfo)) != 0) {
fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
return 1;
}
// loop through all the results and make a socket
for(p = servinfo; p != NULL; p = p->ai_next) {
if ((sockfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) == -1) {
perror("talker client UDP: socket");
continue;
}
break;
}
if (p == NULL) {
fprintf(stderr, "talker client UDP: failed to create socket\n");
return 2;
}
struct sockaddr_in localAddress;

socklen_t addressLength;
addressLength = sizeof(struct sockaddr_in);

if ((numbytes = sendto(sockfd, UDPMessagetoserversA, strlen(UDPMessagetoserversA), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
if ((numbytes = sendto(sockfd, UDPMessagetoserversB, strlen(UDPMessagetoserversB), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
if ((numbytes = sendto(sockfd, UDPMessagetoserversC, strlen(UDPMessagetoserversC), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
if ((numbytes = sendto(sockfd, UDPMessagetoserversD, strlen(UDPMessagetoserversD), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
if((getsockname(sockfd, (struct sockaddr*)&localAddress,&addressLength) == -1))
perror("getsockname() failed in ServerC");

//UDPformatting();
printf("The Client has sent the network topology to the  Server C with UDP port %s number and %s IP address\n",SERVERCUDPPORT,clientIP);
printf("For this connection with Server C, The Client has UDP port \
number %d and IP address %s\n",(int) ntohs(localAddress.sin_port),clientIP);
printf("Server C has this information\n");
printf("Edge====cost\n");
printf("%s",globalmessagebuf);
freeaddrinfo(servinfo);
//printf("talker client UDP: sent %d bytes to %s\n", numbytes,"localhost");
close(sockfd);
return 0;
	
}


/*Sends UDP messages to ServerD*/
int ServerDUDPconnection()
{
int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
//char *msg;
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_DGRAM;
if ((rv = getaddrinfo("localhost", SERVERDUDPPORT, &hints, &servinfo)) != 0) {
fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
return 1;
}
// loop through all the results and make a socket
for(p = servinfo; p != NULL; p = p->ai_next) {
if ((sockfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) == -1) {
perror("talker client UDP: socket");
continue;
}
break;
}
if (p == NULL) {
fprintf(stderr, "talker client UDP: failed to create socket\n");
return 2;
}
struct sockaddr_in localAddress;

socklen_t addressLength;
addressLength = sizeof(struct sockaddr_in);

if ((numbytes = sendto(sockfd, UDPMessagetoserversA, strlen(UDPMessagetoserversA), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
if ((numbytes = sendto(sockfd, UDPMessagetoserversB, strlen(UDPMessagetoserversB), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
if ((numbytes = sendto(sockfd, UDPMessagetoserversC, strlen(UDPMessagetoserversC), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
if ((numbytes = sendto(sockfd, UDPMessagetoserversD, strlen(UDPMessagetoserversD), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);
}
getsockname(sockfd, (struct sockaddr*)&localAddress,&addressLength);

//UDPformatting();
printf("The Client has sent the network topology  to the Server D with UDP port %s number and %s IP address\n",SERVERDUDPPORT,clientIP);
printf("For this connection with Server D, The Client has UDP port \
number %d and IP address %s\n", ntohs(localAddress.sin_port),clientIP);
printf("Server D has this information\n");
printf("Edge====cost\n");
printf("%s",globalmessagebuf);
freeaddrinfo(servinfo);
//printf("talker client UDP: sent %d bytes to %s\n", numbytes,"localhost");
close(sockfd);
return 0;
	
}

/*Sends matrix messages to ServerA*/
void sendadjinfotoserversA()
{
int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
//char *msg;
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_DGRAM;
if ((rv = getaddrinfo("localhost", AdjacancyA, &hints, &servinfo)) != 0) {
fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));

}
// loop through all the results and make a socket
for(p = servinfo; p != NULL; p = p->ai_next) {
if ((sockfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) == -1) {
perror("talker client UDP: socket");
continue;
}
break;
}
if (p == NULL) {
fprintf(stderr, "talker client UDP: failed to create socket\n");

}



if ((numbytes = sendto(sockfd, stringmessage, strlen(stringmessage), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);


}
freeaddrinfo(servinfo);
close(sockfd);
	
}

/*Sends matrix messages to ServerA*/
void sendadjinfotoserversB()
{
int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
//char *msg;
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_DGRAM;
if ((rv = getaddrinfo("localhost", AdjacancyB, &hints, &servinfo)) != 0) {
fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));

}
// loop through all the results and make a socket
for(p = servinfo; p != NULL; p = p->ai_next) {
if ((sockfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) == -1) {
perror("talker client UDP: socket");
continue;
}
break;
}
if (p == NULL) {
fprintf(stderr, "talker client UDP: failed to create socket\n");

}



if ((numbytes = sendto(sockfd, stringmessage, strlen(stringmessage), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);


}
freeaddrinfo(servinfo);
close(sockfd);
	
}

/*Sends matrix messages to ServerC*/
void sendadjinfotoserversC()
{
int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
//char *msg;
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_DGRAM;
if ((rv = getaddrinfo("localhost", AdjacancyC, &hints, &servinfo)) != 0) {
fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));

}
// loop through all the results and make a socket
for(p = servinfo; p != NULL; p = p->ai_next) {
if ((sockfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) == -1) {
perror("talker client UDP: socket");
continue;
}
break;
}
if (p == NULL) {
fprintf(stderr, "talker client UDP: failed to create socket\n");

}



if ((numbytes = sendto(sockfd, stringmessage, strlen(stringmessage), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);


}
freeaddrinfo(servinfo);
close(sockfd);
	
}

/*Sends matrix messages to ServerD*/
void sendadjinfotoserversD()
{
int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
//char *msg;
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_DGRAM;
if ((rv = getaddrinfo("localhost", AdjacancyD, &hints, &servinfo)) != 0) {
fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));

}
// loop through all the results and make a socket
for(p = servinfo; p != NULL; p = p->ai_next) {
if ((sockfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) == -1) {
perror("talker client UDP: socket");
continue;
}
break;
}
if (p == NULL) {
fprintf(stderr, "talker client UDP: failed to create socket\n");

}



if ((numbytes = sendto(sockfd, stringmessage, strlen(stringmessage), 0, p->ai_addr, p->ai_addrlen)) == -1) {
perror("talker client UDP: sendto");
exit(1);


}
freeaddrinfo(servinfo);
close(sockfd);
	
}

/*Make a meaning out of encoded message*/
  void parsedata(char receiveddata[MAXDATASIZE])	
{
       
        char* identifier = strtok(receiveddata,"*");
        
        if (0 == strcmp(identifier,"serverA"))
        {
			spanningtreeA[0] = 0;
            int count = 0;
			char localneighbour[100];
			char localcost[100];
			int money;
		    printf("Neighbour ---Cost\n");
		    receivedinfofromserverA = 1;
		    //printf("%d is flag",receivedinfofromserverA);
		   	while (identifier != NULL)
		   	{    
				 identifier = strtok(NULL,"*"); 
				 
				  if(identifier == NULL)
				  break;
				// printf("%s is identifier\n",identifier);
			     count++;
				 if((strcmp(identifier,"serverB") == 0) || (strcmp(identifier,"serverC") == 0) || (strcmp(identifier,"serverD") == 0))
				 {	 
				  strcpy(localneighbour,identifier);
				 // printf("neighbour info %s\n",localneighbour);
			     }
				 else if(identifier != NULL && atoi(identifier) > 0)
				 {
				  strcpy(localcost,identifier);
				  money = atoi(localcost);
				  //printf("cost info %d\n",money);
			     }
			      
			     if((count%2) == 0) 
			     {
			     printf("%s\t%d\n",localneighbour,money);
			     
			     if(strcmp(localneighbour,"serverB") == 0)
			     {
					 spanningtreeA[1] = money;
				 }
				 if(strcmp(localneighbour,"serverC") == 0)
			     {
					 spanningtreeA[2] = money;
				 }	
				 if(strcmp(localneighbour,"serverD") == 0)
			     {
					 spanningtreeA[3] = money;
				 }
			     append(&serverAlist,localneighbour,money);
			   
			     //matrixappend("A",localneighbour,money);	
			     } 	
			}
			// printList(serverAlist);
			
			
		}
		
		else if( 0 == strcmp(identifier,"serverB"))
		{
		    spanningtreeB[1] = 0;
            int count = 0;
			char localneighbour[100];
			char localcost[100];
			int money;
		    printf("Neighbour----Cost\n");
		    receivedinfofromserverB = 1;
		    //printf("%d is flag",receivedinfofromserverB);
		   	while (identifier != NULL)
		   	{    
				 identifier = strtok(NULL,"*"); 
				 
				 if(identifier == NULL)
				 break;
				// printf("%s is identifier\n",identifier);
			     count++;
				 if((strcmp(identifier,"serverA") == 0) || (strcmp(identifier,"serverC") == 0) || (strcmp(identifier,"serverD") == 0))
				 {	 
				  strcpy(localneighbour,identifier);
				//  printf("neighbour info %s\n",localneighbour);
			     }
				 else if(identifier != NULL && atoi(identifier) > 0)
				 {
				  strcpy(localcost,identifier);
				  money = atoi(localcost);
				 // printf("cost info %d\n",money);
			     }
			      
			     if((count%2) == 0) 
			     {
			     printf("%s\t%d\n",localneighbour,money);	
			      if(strcmp(localneighbour,"serverA") == 0)
			     {
					 spanningtreeB[0] = money;
				 }
				 if(strcmp(localneighbour,"serverC") == 0)
			     {
					 spanningtreeB[2] = money;
				 }	
				 if(strcmp(localneighbour,"serverD") == 0)
			     {
					 spanningtreeB[3] = money;
				 }
			     
			     append(&serverBlist,localneighbour,money);	
			     } 	
			}
			// printList(serverBlist);
				
			
		}
		else if( 0 == strcmp(identifier,"serverC"))
		{
		   spanningtreeC[2] = 0;
            int count = 0;
			char localneighbour[100];
			char localcost[100];
			int money;
		    printf("Neighbour----Cost\n");
		    receivedinfofromserverC = 1;
		   // printf("%d is flag",receivedinfofromserverC);
		   	while (identifier != NULL)
		   	{    
				 identifier = strtok(NULL,"*"); 
				 
				  if(identifier == NULL)
				  break;
				 //printf("%s is identifier\n",identifier);
			     count++;
				 if((strcmp(identifier,"serverD") == 0) || (strcmp(identifier,"serverA") == 0) || (strcmp(identifier,"serverB") == 0))
				 {	 
				  strcpy(localneighbour,identifier);
				  //printf("neighbour info %s\n",localneighbour);
			     }
				 else if(identifier != NULL && atoi(identifier) > 0)
				 {
				  strcpy(localcost,identifier);
				  money = atoi(localcost);
				 // printf("cost info %d\n",money);
			     }
			      
			     if((count%2) == 0) 
			     {
			     printf("%s\t%d\n",localneighbour,money);
			      if(strcmp(localneighbour,"serverA") == 0)
			     {
					 spanningtreeC[0] = money;
				 }
				 if(strcmp(localneighbour,"serverB") == 0)
			     {
					 spanningtreeC[1] = money;
				 }	
				 if(strcmp(localneighbour,"serverD") == 0)
			     {
					 spanningtreeC[3] = money;
				 }
			     	
			     append(&serverClist,localneighbour,money);	
			     } 	
			}
			// printList(serverClist);
			 		
			
		}
		else 
		{
			
		   spanningtreeD[3] = 0;
            int count = 0;
			char localneighbour[100];
			char localcost[100];
			int money;
		    printf("Neighbour----Cost\n");
		     receivedinfofromserverD = 1;
		     //printf("%d is flag",receivedinfofromserverD);
		   	while (identifier != NULL)
		   	{    
				 identifier = strtok(NULL,"*"); 
				 
				  if(identifier == NULL)
				  break;
				// printf("%s is identifier\n",identifier);
			     count++;
				 if((strcmp(identifier,"serverA") == 0) || (strcmp(identifier,"serverC") == 0) || (strcmp(identifier,"serverB") == 0))
				 {	 
				  strcpy(localneighbour,identifier);
				  //printf("neighbour info %s\n",localneighbour);
			     }
				 else if(identifier != NULL && atoi(identifier) > 0)
				 {
				  strcpy(localcost,identifier);
				  money = atoi(localcost);
				//  printf("cost info %d\n",money);
			     }
			      
			     if((count%2) == 0) 
			     {
			     printf("%s\t%d\n",localneighbour,money);
			      if(strcmp(localneighbour,"serverA") == 0)
			     {
					 spanningtreeD[0] = money;
				 }
				 if(strcmp(localneighbour,"serverB") == 0)
			     {
					 spanningtreeD[1] = money;
				 }	
				 if(strcmp(localneighbour,"serverC") == 0)
			     {
					 spanningtreeD[2] = money;
				 }
			     	
			     append(&serverDlist,localneighbour,money);	
			     } 	
			}
			// printList(serverDlist);			
			
		}
		
}       

/*Prints adjacance matrix*/
void adjmatrix()
{
	int i =0,j =0;
 char a[4];
 a[0] = 'A';
 a[1] = 'B';
 a[2] = 'C';
 a[3] = 'D';
 printf("The adjacancy Matrix at client is \n");
 printf("\nserver\tA\tB\tC\tD\n");
 for(i=0;i<4;i++)
 {
	 for(j =0 ;j <4; j++)
	 { 
		 printf("\t%d",adjacancymatrixA[i][j]);
		 
	 }
	      printf("\n%c",a[i]);
		  printf("\n");
		 
		  	
	}
	
	
}

/*converts to strings*/
void converttostring()
{
	char buffer[100];
	int i,j;
	for(i=0;i<4;i++)
{
	for(j=0;j<4;j++)
	{
	sprintf(buffer,"%d",adjacancymatrixA[i][j]);
	strcat(stringmessage,buffer);
    }
}
	
	
	
}

/*Code taken from Google to calculate MST and modified*/
int Cost(int key[], int Set[])
{
  int min = 1000, min_index,v;
  for (v= 0; v < 4; v++)
    if (Set[v] == 0 && key[v] < min)
    min = key[v], min_index = v;
    return min_index;
}

/*Code taken from Google  to calculate MST and modified*/
void CalculateMST(int previous[], int n, int local[4][4])
{
  int i;
  char *printvalpre;
  char *printvali;
  printf(" Minimum spanning Tree obtained is\n");
  
  for (i = 1; i < 4; i++)
  {
    mCost=local[i][previous[i]]+mCost; 
    if(previous[i] == 0)
    printvalpre = "serverA";
    else if(previous[i] == 1)
    printvalpre = "serverB";
    if(previous[i] == 2)
    printvalpre = "serverC";
    else if(previous[i] == 3)
    printvalpre = "serverD";
     if(i == 0)
    printvali = "serverA";
    else if(i == 1)
    printvali = "serverB";
    if(i == 2)
    printvali = "serverC";
    else if(i == 3)
    printvali = "serverD";
    printf("%s ---------------------- %s %d \n", printvalpre, printvali, local[i][previous[i]]);
    //printf("%d - %d\n", previous[i], i);
  }
  printf("Minimum  cost obtained  %d\n",mCost);
}


/*Code taken from Google to calculate MST and modified*/
void MST()
{
  int previous[4];
  int key[4]; 
  int mstSet[4]; 
  int count=0,v;
  int local[4][4];
  int j,u,l;


  for(l=0;l<4;l++)
    for(j=0;j<4;j++)
      local[l][j]=temp[l][j];

  for (l = 0; l < 4; l++)
    key[l] = 1000, mstSet[l] = 0;


  key[0] = 0;  
  previous[0] = -1; 

  for (count = 0; count < 4-1; count++)
  {
    u = Cost(key, mstSet);
    mstSet[u] = 1;
    for (v = 0; v < 4; v++)
    if (local[u][v] && mstSet[v] == 0 && local[u][v] < key[v])
      previous[v] = u, key[v] = local[u][v];
  }
  CalculateMST(previous, 4, local);
}



void formatmessagetoMST()
{
   	int i,j;
   	for(i=0;i<4;i++)
   	{
		for(j=0;j<4;j++)
		{
			if(i==0)
			{
				temp[0][j] = spanningtreeA[j];
				}
				if(i==1)
			  {
				temp[1][j] = spanningtreeB[j];
				}
				if(i==2)
			   {
				temp[2][j] = spanningtreeC[j];
				}
				if(i==3)
			   {
				temp[3][j] = spanningtreeD[j];
				}
			}
		}
	//for(i=0;i<4;i++)
	//{
		//for(j=0;j<4;j++)
		//{
			//printf("%d ",temp[i][j]);
			
			//}
			//printf("\n");
		//}
	
}



int main(void)
{
	
	/*Code taken from Beejs tutorial for socket programming and modified*/
	
	
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	char receiveddata[MAXLENGTH];
	char comparedata[MAXLENGTH];
	int yes=1;
	char buf[MAXLENGTH];
	int numbytes;
	char s[INET6_ADDRSTRLEN];
	int rv;
	
	
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo("localhost", CLIENTTCPPORT, &hints, &servinfo)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
	}
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
	if ((sockfd = socket(p->ai_family, p->ai_socktype,
	p->ai_protocol)) == -1) {
	perror("client: socket");
	continue;
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
	sizeof(int)) == -1) {
	perror("setsockopt");
	exit(1);
	}
	if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	close(sockfd);
	perror("client: bind");
	continue;
	}
	
		//local IP Adress

		void *addr;
		char *ipver;

		// get the pointer to the address itself,
		// different fields in IPv4 and IPv6:
		if (p->ai_family == AF_INET) { // IPv4
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
		addr = &(ipv4->sin_addr);
		ipver = "IPv4";
		} else { // IPv6
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
		addr = &(ipv6->sin6_addr);
		ipver = "IPv6";
		}

		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, clientIP, sizeof clientIP);
	break;
	}
	struct sockaddr_in localAddress;
    socklen_t addressLength;
    addressLength = sizeof(struct sockaddr_in);
    getsockname(sockfd, (struct sockaddr*)&localAddress,&addressLength);
    //inet_ntop(localAddress.ss_family, get_in_addr((struct sockaddr *)&localAddress), s, sizeof s);
    printf("The Client has TCP port number %s and IP address %s\n",CLIENTTCPPORT,clientIP);
    
	freeaddrinfo(servinfo); //all done with this structure
	
	if (p == NULL) {
	fprintf(stderr, "client: failed to bind\n");
	exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1) {
	perror("listen");
	exit(1);
	}
	
	//printf("client: waiting for connections...\n");
	while(1) { // main accept() loop
	sin_size = sizeof their_addr;
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if (new_fd == -1) {
	perror("accept");
	continue;
	}
	inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
	
//	if (!fork()) { // this is the child process
//	close(sockfd); // child doesn't need the listener
	if ((numbytes = recv(new_fd, buf, MAXLENGTH-1, 0)) == -1)
	{
	perror("recv");
	exit(1);
	}
	buf[numbytes] = '\0';
	
	
	struct sockaddr_in localAddress1;
	struct sockaddr_in localAddress;
    socklen_t addressLength,addressLength1;
    addressLength = sizeof(struct sockaddr_in);
    addressLength1 = sizeof(struct sockaddr_in);
    getsockname(sockfd, (struct sockaddr*)&localAddress,&addressLength);
    getpeername(new_fd, (struct sockaddr*)&localAddress1,&addressLength1);  
    /*Code taken from Beejs tutorial for socket programming ends here*/
    
	//Parsing data received
	strcpy(receiveddata,buf);
	strcpy(comparedata,buf);
		
	char *local =NULL;
	local= strtok(comparedata,"*");
	
	//local= strtok(comparedata,"*");
	if(strcmp(local,"serverA") == 0 )
	{
	   printf("	The Client receives neighbor information from the ServerA with TCP port number %d and IP address %s\n",(int) ntohs(localAddress1.sin_port),s);
	   printf("For this connection with Server A, The Client has TCP port number %s IP address %s\n",CLIENTTCPPORT, clientIP);
	   parsedata(receiveddata);
    }
	else if(strcmp(local,"serverB") ==0 )
	{
	printf("The Client receives neighbor information from the ServerB with TCP port number %d and IP address %s\n",(int) ntohs(localAddress1.sin_port),s);
	 printf("For this connection with Server B, The Client has TCP port number %s IP address %s\n",CLIENTTCPPORT, clientIP);
		 parsedata(receiveddata);
	}
	else if(strcmp(local,"serverC") ==0 )
	{
		printf("The Client receives neighbor information from the ServerC with TCP port number %d and IP address %s\n",(int) ntohs(localAddress1.sin_port),s);
		 printf("For this connection with Server C, The Client has TCP port number %s IP address %s\n",CLIENTTCPPORT, clientIP);
		 parsedata(receiveddata);
	}
	else if(strcmp(local,"serverD") ==0 )
	{ 
		printf("The Client receives neighbor information from the ServerD with TCP port number %d and IP address %s\n",(int) ntohs(localAddress1.sin_port),s);
		 printf("For this connection with Server D, The Client has TCP port number %s IP address %s\n",CLIENTTCPPORT, clientIP);
		parsedata(receiveddata);
		
	}
	//parsedata(receiveddata);
	
	

	
	//close(new_fd);
	//exit(0);
	//}
	//close(new_fd); // parent doesn't need this
	if( receivedinfofromserverA &&  receivedinfofromserverB &&  receivedinfofromserverC &&  receivedinfofromserverD)
		{
		//printf("calling client UDP\n");
		UDPformatting();
		ServerAUDPconnection();	
		ServerBUDPconnection();
		ServerCUDPconnection();
		ServerDUDPconnection();
		converttostring();
		sendadjinfotoserversA();
		sendadjinfotoserversB();
		sendadjinfotoserversC();
		sendadjinfotoserversD();
		adjmatrix();
		
		//for( i=0;i<4;i++)
		//{
		//printf("%d Ais spanning tree\n",spanningtreeA[i]);
		//printf("%d Bis spanning tree\n",spanningtreeB[i]);
		//printf("%d Cis spanning tree\n",spanningtreeC[i]);
		//printf("%d Dis spanning tree\n",spanningtreeD[i]);
	   //}
	     formatmessagetoMST();
	     //spanningtreenew(4);
	     MST();
	     
	     
		}
		
	}
	
	return 0;
}
	
	
	
