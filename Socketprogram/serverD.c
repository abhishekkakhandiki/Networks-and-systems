#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include "utils.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
if (sa->sa_family == AF_INET) {
return &(((struct sockaddr_in*)sa)->sin_addr);
}
return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


/*Global variabels*/
int adjacancymatrixA[4][4];
char *serverAneighbours[10];
char *serverAcosts[10];
char serverBneighbours[100];
char serverBcosts[100];
char serverCneighbours[100];
char serverCcosts[100];
char serverDneighbours[100];
char serverDcosts[100];
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
char Ipadressforthisserver[1024];
char sendbuffer[MAXLENGTH];
char msg[MAXLENGTH];


/*Function to read a file*/
void fileread()
{

	FILE *fp;
	char neighbours [MAXLENGTH];
	char cost [MAXLENGTH];
	//char msg[MAXLENGTH];
	 
	fp = fopen("serverD.txt","r");
	if(fp == NULL)
	{
	printf("error opening file\n");
	exit(-1);
	}
	printf("The ServerD is up and running\n");
	strcat(msg,"serverD");
	//strcat(msg,"END");
    printf("The server D has following neighbour information \nNeighbour ----- cost\n");
	fscanf(fp,"%s %s",neighbours ,cost);
	printf("%s\t\t%s\n",neighbours,cost);
	
	while(!feof(fp))
	{	
		strcat(msg,"*");
		strcat(msg,neighbours);
		strcat(msg,"*");
		strcat(msg,cost);
		strcat(msg,"*");
		//strcat(msg,"END");
		fscanf(fp,"%s %s",neighbours ,cost);
		if(feof(fp))
		break;
		printf("%s\t\t%s\n",neighbours,cost);
						
	}
	
	//printf("%s %s\n",neighbours,cost);
	//printf("%s is msg sent " , msg);
	
 fclose(fp);
 //strcat(sendbuffer,msg);
 //printf("%s is sendbuffer sent " , sendbuffer);
 //return msg;	
}


/*Function that makes a meaning from UDP data received from client*/
void stripUDPdata(char received[MAXBUFLEN])
{
	char local[MAXBUFLEN];
	char temp[20];
	char *token;
	//char *test;
	//int b,c,d;
	//int i;
	
	
	
	strcpy(local,received);
	//printf("%s this is received\n",local);
	token=strtok(local,"*");
	
	if(strcmp(token,"A") == 0)
	{
		
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
				  printf("\nAB\t");
				  countAB++;
		} 
		else if(strcmp(temp,"serverC")==0)
	  	{
				//printf("%s is token in A\n",token);
				 
			      printf("\nAC\t"); 
			      countAC++;
				adjacancymatrixA[0][2] = 1;
		} 
		else if(strcmp(temp,"serverD")==0)
	  	{
				//printf("%s is token in A\n",token);
				
				// printf("%s is serverAneighbours[1] \n",serverAneighbours[2]); 
				adjacancymatrixA[0][3] = 1;		
				printf("\nAD\t");
				countAD++;
	    }
	    else
	    { 
		  //printf("%s is token in A\n",token);	
		  if(token!=NULL)	
		  printf("%s\n",token);
		} 	  	  	
	}
		
    }
    else if(strcmp(token,"B") == 0)
    {
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
			flagforB = 1;	
			printf("\nBA\t");
			flagforB =1;
			countAB++;
		   }
	   	   //printf("%s is token B\n",token);
	    }
	    else if(strcmp(temp,"serverC") ==0)
	  	{
			  adjacancymatrixA[1][2] = 1;
			   printf("\nBC\t");
			   countBC++;
			   forthiscaseB =1;	
	  	   //printf("%s is token B\n",token);
	    }
	    else if(strcmp(temp,"serverD") ==0)
	  	{
			adjacancymatrixA[1][3] = 1;
			   	printf("\nBD\t");
			   	countBD++;
			   	forthiscaseB =1;
	  	  //printf("%s is token B\n",token);
	    }
	    else
	    { 
			if((token!=NULL && flagforB) || (token!=NULL && forthiscaseB == 1))
			{
				forthiscaseB = 0;
				flagforB = 0;
		    printf("%s\n",token);
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
			printf("\nCA\t");
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
			printf("\nCB\t");
			countBC++;
			flagforC = 1;
		   }
	  //	printf("%s is token C\n",token);
	    }
	    else if(strcmp(temp,"serverD") == 0)
	  	{	  	
		 adjacancymatrixA[2][3] = 1;
		  	printf("\nCD\t");
		  	countCD++;
		  	forthiscaseC =1;
	  //	printf("%s is token C\n",token);
	    }
	    else
	    {
		   if((token!=NULL && flagforC) || (token!=NULL && forthiscaseC) )
		   {
			   flagforC = 0;
			   forthiscaseC = 0;
		   printf("%s\n",token);
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
		  printf("\nDA\t");
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
		  printf("\nDB\t");
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
		  printf("\nDC\t");
		  countCD++;
		  flagforD = 1;
	     }
	  	//printf("%s is token D\n",token);
	    }
	    else
	    {
		 if(token!=NULL && flagforD)
		 {
			 flagforD = 0;
		printf("%s\n",token);
	    }
		}	  	
	  	//printf("%s is token D\n",token);	  	
	}	
		
	}
	
}

void printadjacancymatrix()
{
 int i =0,j =0;
 char a[4];
 a[0] = 'A';
 a[1] = 'B';
 a[2] = 'C';
 a[3] = 'D';
 printf("The adjacancy Matrix for server D is \n");
 printf("\nserver\tA\tB\tC\tD\n");
 for(i=0;i<4;i++)
 {
	 for(j =0 ;j <4; j++)
	 { 
		 printf("\t%c",adjacancymatrixA[i][j]);
		 
	 }
	      printf("\n%c",a[i]);
		  printf("\n");
		 
		  	
	}
}

int ServerUDPFunctionality()
{ 
	

int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
struct sockaddr_storage their_addr;
char buf[MAXBUFLEN];
char receivedUDPAdata[MAXBUFLEN];
char receivedUDPBdata[MAXBUFLEN];
char receivedUDPCdata[MAXBUFLEN];
char receivedUDPDdata[MAXBUFLEN];
socklen_t addr_len;
char s[INET6_ADDRSTRLEN];
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
hints.ai_socktype = SOCK_DGRAM;
hints.ai_flags = AI_PASSIVE; // use my IP
if ((rv = getaddrinfo("localhost", SERVERDUDPPORT, &hints, &servinfo)) != 0) {
fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
return 1;
}
// loop through all the results and bind to the first we can
for(p = servinfo; p != NULL; p = p->ai_next) {
if ((sockfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) == -1) {
perror("listener UDP: socket");
continue;
}
if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
close(sockfd);
perror("listener UDP: bind");
continue;
}
break;
}
if (p == NULL) {
fprintf(stderr, "listener UDP : failed to bind socket\n");
return 2;
}
freeaddrinfo(servinfo);
//printf("Server B: waiting to recvfrom...\n");
addr_len = sizeof their_addr;


//serverA
if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
(struct sockaddr *)&their_addr, &addr_len)) == -1) {
perror("recvfrom");
exit(1);
}

//struct sockaddr_in localAddress;
//struct sockaddr_in localAddress1;
//socklen_t addressLength;
//addressLength = sizeof(struct sockaddr_in);
//getsockname(sockfd, (struct sockaddr*)&localAddress,&addressLength);
//getpeername(sockfd, (struct sockaddr*)&localAddress1,&addressLength);
     //printf("local address: %s\n", inet_ntoa( localAddress.sin_addr));
     //printf("local port: %d\n", (int) ntohs(localAddress.sin_port));
//printf("listener: got packet from %s\n",inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
struct sockaddr_in *clientport = (struct sockaddr_in *)malloc(sizeof (struct sockaddr_in));
clientport=&their_addr;
printf("The server D has received the network topology from the Client with UDP port number %d and IP address %s\n",(int) ntohs(clientport->sin_port),inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));

printf("For this connection with Client, The Server D has %s UDP port number and %s IP address\n",SERVERDUDPPORT,Ipadressforthisserver);

printf("Edge====Cost\n");

buf[numbytes] = '\0';
strcpy(receivedUDPAdata,buf);
stripUDPdata(receivedUDPAdata);
//printf("listener: packet is %d bytes long\n", numbytes);

//printf("listener: packet contains \"%s\"\n", buf);

//serverB

if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
(struct sockaddr *)&their_addr, &addr_len)) == -1) {
perror("recvfrom");
exit(1);
}
//printf("listener: got packet from %s\n",
//inet_ntop(their_addr.ss_family,
//get_in_addr((struct sockaddr *)&their_addr),
//s, sizeof s));
buf[numbytes] = '\0';
strcpy(receivedUDPBdata,buf);
stripUDPdata(receivedUDPBdata);
//printf("listener: packet is %d bytes long\n", numbytes);

//printf("listener: packet contains \"%s\"\n", buf);

//serverc

if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
(struct sockaddr *)&their_addr, &addr_len)) == -1) {
perror("recvfrom");
exit(1);
}
//printf("listener: got packet from %s\n",
//inet_ntop(their_addr.ss_family,
//get_in_addr((struct sockaddr *)&their_addr),
//s, sizeof s));
buf[numbytes] = '\0';
strcpy(receivedUDPCdata,buf);
stripUDPdata(receivedUDPCdata);
//printf("listener: packet is %d bytes long\n", numbytes);

//printf("listener: packet contains \"%s\"\n", buf);

//serverd


if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
(struct sockaddr *)&their_addr, &addr_len)) == -1) {
perror("recvfrom");
exit(1);
}
//printf("listener: got packet from %s\n",
//inet_ntop(their_addr.ss_family,
//get_in_addr((struct sockaddr *)&their_addr),
//s, sizeof s));
buf[numbytes] = '\0';
strcpy(receivedUDPDdata,buf);
stripUDPdata(receivedUDPDdata);
//printf("listener: packet is %d bytes long\n", numbytes);

//printf("listener: packet contains \"%s\"\n", buf);


//formatmsg();

close(sockfd);
return 0;
}

void ReceiveADJinfo()
 
 {
	 int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv,i,j;
int temp[4][4];
int numbytes;
struct sockaddr_storage their_addr;
char buf[MAXBUFLEN];
char receivedmatrixinfo[MAXBUFLEN];

socklen_t addr_len;
char s[INET6_ADDRSTRLEN];
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
hints.ai_socktype = SOCK_DGRAM;
hints.ai_flags = AI_PASSIVE; // use my IP
if ((rv = getaddrinfo("localhost", AdjacancyD, &hints, &servinfo)) != 0) {
fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
return 1;
}
// loop through all the results and bind to the first we can
for(p = servinfo; p != NULL; p = p->ai_next) {
if ((sockfd = socket(p->ai_family, p->ai_socktype,
p->ai_protocol)) == -1) {
perror("listener UDP: socket");
continue;
}
if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
close(sockfd);
perror("listener UDP: bind");
continue;
}
break;
}
if (p == NULL) {
fprintf(stderr, "listener UDP : failed to bind socket\n");
return 2;
}
freeaddrinfo(servinfo);
//printf("Server B: waiting to recvfrom...\n");
addr_len = sizeof their_addr;


//serverA
if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
(struct sockaddr *)&their_addr, &addr_len)) == -1) {
perror("recvfrom");
exit(1);
}
strcpy(receivedmatrixinfo,buf);
for(i=0;i<16;i++)
{
for (j = 0; j<4;j++)
{
	adjacancymatrixA[i][j] = receivedmatrixinfo[4*i+j];
}
}

close(sockfd);
return 0;
	 	 
}
int main()
{
	
	//char* message;
	int numbytes;
	//char sendbuffer[MAXLENGTH];
    //message = fileread();
    //strcpy(sendbuffer,message);
    //Socket functionality TCP
   fileread();
    int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo("localhost", SERVERATCPPORT, &hints, &servinfo)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
	}
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
	if ((sockfd = socket(p->ai_family, p->ai_socktype,
	p->ai_protocol)) == -1) {
	perror("ServerD: socket");
	continue;
	}
	if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	close(sockfd);
	perror("ServerD: connect");
	continue;
	}
	break;
	}
	if (p == NULL) {
	fprintf(stderr, "ServerD: failed to connect\n");
	return 2;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s,sizeof s);
	//printf("Server: connecting to %s\n", s);
	freeaddrinfo(servinfo); // all done with this structure
	if ((numbytes=send(sockfd,msg,strlen(msg),0)) == -1) {
	perror("send");
	exit(1);
	}
	//printf("%s is sent data",sendbuffer);
	 struct sockaddr_in localAddress;
     socklen_t addressLength;
     addressLength = sizeof(struct sockaddr_in);
     getsockname(sockfd, (struct sockaddr*)&localAddress,&addressLength);
     //printf("local address: %s\n", inet_ntoa( localAddress.sin_addr));
     //printf("local port: %d\n", (int) ntohs(localAddress.sin_port));
     //printf("%s is send buffer\n",msg); 
	  printf("The Server D finishes sending its neighbor information to the Client with TCP port number %s and IP address %s\n",CLIENTTCPPORT,s);
	  printf("For this connection with the Client, the Server D has TCP port number %d and IP address %s\n ",(int) ntohs(localAddress.sin_port),s);
	  strcpy(Ipadressforthisserver,s);
	  close(sockfd);
	
	//server functionality UDP
	 ServerUDPFunctionality();
	 ReceiveADJinfo();
	 //adjacancy matrix
	 printadjacancymatrix();
	
	
 return 0;
}
