

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "my402list.h"

int B = 10;
int n = 20;
int P = 3;
int i = 0;
double r =1.5;
double lambda = 1;
double mu = 0.5;
int flag = 0;
int shutdownservers = 0;
char filename[100];
int tracefileflag = 0;
int droppacket = 0;
int totalpacketgenerated;
int tockenbucketcount ;
int endpacketthread ;
int endpacketthreadcntc = 0 ;
int endtokenthreadcntc = 0;
int endtokenthread ;
int endserver1 = 0;
int endserver2 = 0;
int packetdropflag = 0;
int tokendropflag = 0;
int controlcremovedpackets = 0;
int controlc = 0;
int endserverthreads = 0;
long double totaltimeinpacketqueue = 0;
long double totaltimeinserverqueue = 0;
long double totaltimeins1 = 0;
long double totaltimeins2 = 0;
int droptokenCount = 0;
int totaltokencount = 0;
long double  totalinterarrivaltime;
long double totalservicetime;
int totalpacketsserviced;
long double totaltimeinsystem;
long double totalsquaretime;
struct timeval emulationstart, emulationend;
long double totalemulation;
pthread_t tokendepositingthread;
pthread_t firstserverthread;
pthread_t secondserverthread;
pthread_t packetarrivalthread;
pthread_t signalthread;
sigset_t set;

My402List* arrivalqueue;
My402List* departurequeue;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t out = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Q2empty = PTHREAD_COND_INITIALIZER;

typedef struct packetStruct{
	int packetnumber;
	int tokensrequired;
	double servicetime;
	struct timeval Q1arrivalTime;
	struct timeval Q1departureTime;
	struct timeval Q2arrivalTime;
	struct timeval Q2departureTime;
} Packet;

typedef struct input
{
	int tokensrequired;
    int servicetime;
	int interarrivaltime;
	
}input;


int isDirectory(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

input* readlinebyline(FILE *p , int linenumber);
void readcommandline(int argc,char*argv[]);
void displayinputparameters();
void setdefaultvalues();
void generatestatisctics();
void removepacketsfromq1q2();

int q2movement(){
       
		if(!My402ListEmpty(arrivalqueue)){
			My402ListElem *first = My402ListFirst(arrivalqueue);
			if(((Packet *)(first -> obj)) -> tokensrequired <= tockenbucketcount)
				return 1;
		}
		
			return 0;
         
}

input* readlinebyline(FILE* fp, int a)
{
	int interarrivaltime,servicetime;
	int tokensrequired;
	input * temp = (input*)malloc(sizeof(input));
	char* value = NULL ;
	char buf[100];
	if(fgets(buf,100,fp)!=NULL)
	{
		buf[strlen(buf) - 1] = '\0';
		value = strtok(buf," \t");
		if(value!=NULL)
		interarrivaltime = atoi(value);
		
		value = strtok(NULL," \t");
		if(value!=NULL)
		tokensrequired = atoi(value);
		value = strtok(NULL," \t");
		
		if(value!=NULL)
	    servicetime = atoi(value);
		value = strtok(NULL," \t");
		
		if(value == NULL)
	    {
			temp -> tokensrequired = tokensrequired;
			temp -> servicetime = servicetime;
			temp -> interarrivaltime = interarrivaltime;
			
			
		}
return(temp);
		
	}
	
	return(0);
	
}
void setdefaultvalues()
{
  B =10;
  n =20;
  P =3;
  r =1.5;
  lambda =1;
  mu = 0.5;	
	
}

void removepacketsfromq1q2()

{
	int q1length = 0;
	int q2length = 0;
	int i,j;
	int number;
	 q1length = My402ListLength(arrivalqueue);
	// printf("%d is q1 length\n",q1length);
	 q2length = My402ListLength(departurequeue);
	 
	 controlcremovedpackets = q1length+q2length;
	 
	for(i = 0 ; i < q1length; i++)
	{
	   	My402ListElem *q1packet = My402ListFirst(arrivalqueue);
	   	number = ((Packet*)(q1packet->obj)) -> packetnumber;
	   //	((Packet *)(first -> obj)) -> packetnumber
	   	//printf("%d is packet number\n ***",number);
	   printf("p%d removed from Q1\n",number);
	   My402ListUnlink(arrivalqueue,q1packet);
	   	
	} 
	
	for(j = 0 ; j < q2length; j++)
	{
	   	My402ListElem *q2packet = My402ListFirst(departurequeue);
	   	number = ((Packet*)(q2packet->obj)) -> packetnumber;
	   //	((Packet *)(first -> obj)) -> packetnumber
	   	//printf("%d is packet number\n ***",number);
	   printf("p%d removed from Q2\n",number);
	   My402ListUnlink(departurequeue,q2packet);
	   	
	} 
	
 return ;	
		
	
}

void readcommandline(int argc,char* argv[])
{
	 int i;
    char *usageinformation = "warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]";
    int direct ;
	 for ( i=1 ;i < argc; i=i+2)
{	
	if(0==strcmp(argv[i],"-lambda"))
	{
		if((argv[i+1]) == NULL)
		{
		fprintf(stdout,"Usage information = %s\n",usageinformation);
		exit(0) ;
	    }
		lambda=atof(argv[i+1]);
		if(1/lambda > 10)
		{
			lambda = 0.1;
		}	
			
	}
	else if(0==strcmp(argv[i],"-mu"))
	{
		if((argv[i+1]) == NULL)
		{
		fprintf(stdout,"Usage information = %s\n",usageinformation);
		exit(0) ;
	    }
		
		mu =atof(argv[i+1]);
		if(1/mu> 10)
		{
		   mu = 0.1;
		}
				
	}
	
	 else if(0==strcmp(argv[i],"-r"))
	{	
		if((argv[i+1]) == NULL)
		{
		fprintf(stdout,"Usage information = %s\n",usageinformation);
		exit(0) ;
	    }
		r =atof(argv[i+1]);	
		if(1/r > 10)
		{
			r = 0.1;
			
		}	
	}

	
	else if(0==strcmp(argv[i],"-B"))
	{
		
		if((argv[i+1]) == NULL)
		{
		fprintf(stdout,"Usage information = %s\n",usageinformation);
		exit(0) ;
	    }
    	B=atoi(argv[i+1]);				
	}
	
    else if(0==strcmp(argv[i],"-P"))
	{	
		if((argv[i+1]) == NULL)
		{
		fprintf(stdout,"Usage information = %s\n",usageinformation);
		exit(0) ;
	    }
		P =atoi(argv[i+1]);	
	}
	else if(0==strcmp(argv[i],"-n"))
	{	
		if((argv[i+1]) == NULL)
		{
		fprintf(stdout,"Usage information = %s\n",usageinformation);
		exit(0) ;
	    }
		n=atoi(argv[i+1]);	
	}
	
	else if(0==strcmp(argv[i],"-t"))
	{ 
		     FILE *fp=NULL;
		
		  direct = isDirectory(argv[i+1]);
          
          if(direct == 1)
          {
			  fprintf(stderr,"Reading a directory and not a file\n");
			  exit(0);
		  }
		  if(-1 == access( argv[i+1], F_OK ))
             {
				perror("Error:");
				exit(0);
               } 
                  
		if((argv[i+1]) == NULL)
		{
		fprintf(stdout,"Usage information = %s\n",usageinformation);
		exit(0) ;
	    }  
	      fp = fopen(argv[i+1], "r");
	    if( !fp )
            {
				perror("Input file cannot be opened");
				exit(0);
				fclose(fp);
			}  
             tracefileflag = 1;
             strcpy(filename,argv[i+1]);                
	} 
	
	else
	{
		fprintf(stdout,"Malformed command\n");
		fprintf(stdout,"Usage information : %s\n",usageinformation);
		setdefaultvalues();
		exit(1);
	}
	
		
}		
	
}


void displayinputparameters()
{
	FILE *file1 = fopen(filename, "r");	
	char lines[100];
	
	if(tracefileflag){
		fgets(lines,sizeof(lines),file1);
	    n = atoi(lines);
	    if(n == 0)
	    {
	    printf("Malformed command :Input file format is not proper \n");
	    exit(0);
	   }
		fclose(file1);
	}
	printf("\nEmulation Parameters:\n");
	printf("\n\tnumber to arrive = %d", n);
	if(!tracefileflag)
    printf("\n\tlambda = %.3g", lambda);
	if(!tracefileflag)
	printf("\n\tmu = %.3g", mu);
	printf("\n\tr = %.3g", r);
	printf("\n\tB = %d", B);
	if(!tracefileflag)
	printf("\n\tP = %d", P);
	if(tracefileflag)
	printf("\n\ttsfile = %s", filename);
	printf("\n\n");
	
	
}
void initializequeues()
{
	    arrivalqueue = (My402List *)malloc(sizeof(My402List));
        
		My402ListInit(arrivalqueue);
		
		departurequeue = (My402List *)malloc(sizeof(My402List));
		
		My402ListInit(departurequeue);
				
}

void addtopacketqueue(My402List*list,Packet* new)
{
   if(arrivalqueue!=NULL)
   {
	 My402ListAppend(list,new);      
   }	
	
}

long double microseconds(struct timeval t)

{
	 return ((t.tv_sec*1000000)+t.tv_usec);
	
}

long double printtime()

{
    struct timeval currtime;
    gettimeofday(&currtime,0);
    return(microseconds(currtime) - microseconds(emulationstart))/1000.0;	
		
}


void generatestatisctics()
{
	long double avgnoofpacketsq1,avgnoofpacketsq2,avgnoofpacketss1,avgnoofpacketss2,tokendropprobability,packetdropprobability;
	long double avgpacketinterarrivaltime,avgpacketservicetime,avgtimespentinsystem;
	long double p1=0,p2=0,p3=0,p4=0;
	long  double stddev = 0;
	long double temp;
	
	if((totalpacketgenerated-droppacket-controlcremovedpackets) == 0 || (totalpacketgenerated-droppacket-controlcremovedpackets) < 0)
	temp = n;
	else
	temp = (totalpacketgenerated-droppacket-controlcremovedpackets);
	if(totalpacketgenerated < 0 || totalpacketgenerated == 0)
	totalpacketgenerated = n;
	if(totaltokencount < 0 || totaltokencount == 0)
	totaltokencount = n;
	 
	avgpacketinterarrivaltime = (totalinterarrivaltime/1000)/totalpacketgenerated;
	//printf("total packets gen %d\n",totalpacketgenerated);
	//printf("%Lf is total interarrival time\n ",totalinterarrivaltime);
	avgpacketservicetime = (totalservicetime/1000)/temp;
	
	
	tokendropprobability = (double)droptokenCount/(double)totaltokencount;
	packetdropprobability = (double)droppacket/(double)totalpacketgenerated;
	avgnoofpacketsq1 = totaltimeinpacketqueue/totalemulation;
	avgnoofpacketsq2 = totaltimeinserverqueue/totalemulation;
	avgnoofpacketss1 = totaltimeins1/totalemulation;
	avgnoofpacketss2 = totaltimeins2/totalemulation;
	
	avgtimespentinsystem = (totaltimeinsystem/1000)/temp;
	
	
	p1 = totalsquaretime /(1000*1000);
	
	p2 = temp;
	
	p3 = p1/p2;
	
	p4 = avgtimespentinsystem*avgtimespentinsystem;
	
	stddev = sqrt(p3-p4);
	printf("\n\nStatistics:\n\n");
	
	printf("\taverage packet inter-arrival-time = %.6Lf\n",avgpacketinterarrivaltime);
	
	printf("\taverage packet service-time = %.6Lf\n\n",avgpacketservicetime);
    
	
	printf("\taverage number of packets in Q1 = %.6Lf\n",avgnoofpacketsq1);
	printf("\taverage number of packets in Q2 = %.6Lf\n",avgnoofpacketsq2);
	printf("\taverage number of packets in S1 = %.6Lf\n",avgnoofpacketss1);
	printf("\taverage number of packets in S2 = %.6Lf\n\n",avgnoofpacketss2);
	
	printf("\taverage time a packet spent in  system = %.6Lf\n",avgtimespentinsystem);
	printf("\tstandard deviation for time spent in system = %.6Lf\n\n",stddev);
   
	
	printf("\ttoken drop probability = %.6Lf\n",tokendropprobability);
	printf("\tpacket drop probability = %.6Lf\n\n",packetdropprobability);

		
}


void* processsignal(void* arg)
{
 int sig=0;
 
  sigwait(&set, &sig);
  
 
  
 pthread_mutex_lock(&m);
 

  controlc = 1;
  endtokenthreadcntc = 1;
  endpacketthreadcntc = 1;
  endserverthreads = 1;
  
  pthread_cancel(packetarrivalthread);
  pthread_cancel(tokendepositingthread);
  pthread_cond_broadcast(&Q2empty);
  removepacketsfromq1q2();
  
  pthread_mutex_unlock(&m);
  
  
  return 0;	
}


void* processpacket(void* arg)
{
	long double interarrivaltime,servicetime,displaytime,actualinterarrivaltime,sleeptime;
	long double timeinq1;
	struct timeval currenttime,timeaftersleep,previousfinish,previousstart;
	int tokensrequired,packetcounter = 0;
	char lines[100];
	int totallines;
	input* value = NULL;
	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
	
	FILE* fp = fopen(filename,"r");
	
	if(controlc)
	fclose(fp);
	
	if(tracefileflag)
	{
		
		fgets(lines,sizeof(lines),fp);
		n = atoi(lines);
		
	}
	totallines = n;
	while(!controlc)
	{	 	
		   if(tracefileflag)
		   {
		      if(totallines > 0)
		      {
				  value =  readlinebyline(fp,2);
				  interarrivaltime = value->interarrivaltime;
				  servicetime = value -> servicetime;
				  tokensrequired = value->tokensrequired;
				  totallines--;
				  
				 }
			}
		/*File reading*/
	  	
	else
	{
		interarrivaltime = 1000.0/lambda;
	    servicetime = 1000.0/mu;	
		tokensrequired = P;
		
	}
	
	gettimeofday(&previousfinish,0);
	if(packetcounter == 0)
	{
		
	usleep(interarrivaltime*1000);
	if(controlc)
	return 0 ;
	
    }
	if (packetcounter != 0)
	{
		sleeptime = interarrivaltime*1000-(microseconds(previousfinish)-microseconds(previousstart));
		if(sleeptime < 0)
		{
		
	    usleep(0);
	    if(controlc)
	    return 0 ;
	   
	    }
	    else
	    {
		
	    usleep(sleeptime) ;
	    if(controlc)
	    return 0 ;
	   
	    }
    }
	gettimeofday(&timeaftersleep,0);
	if(packetcounter == 0)
	{
		actualinterarrivaltime = interarrivaltime;
		
		
	}
	else
	{
	actualinterarrivaltime = (microseconds(timeaftersleep) - microseconds(previousstart))/1000;
	
    }
    
    
	if(packetcounter + 1 >  n){
		 
			endpacketthread = 1;
			break;
			//return 0;
		}
	totalinterarrivaltime+=actualinterarrivaltime;
	
	Packet* p = (Packet*)malloc(sizeof(Packet));
	packetcounter++;
	totalpacketgenerated++;
	p->tokensrequired = tokensrequired;
	p->servicetime = servicetime;
	p->packetnumber = packetcounter;
	
	if(p->tokensrequired <= B)
	{
	gettimeofday(&currenttime,0);
	p->Q1arrivalTime = currenttime;
	displaytime = printtime();
	//flockfile(stdout);
	printf("%012.3Lfms: p%d arrives, needs %d tokens, inter-arrival time = %.3Lfms\n",displaytime,p->packetnumber,p->tokensrequired,actualinterarrivaltime);
	//funlockfile(stdout);
   }
	
	else 	
	{
		droppacket++;
		displaytime = printtime();
		gettimeofday(&currenttime,0);
		p->Q1arrivalTime = currenttime;
		gettimeofday(&previousstart,0);
		//flockfile(stdout);
		printf("%012.3Lfms: p%d arrives, needs %d tokens, inter-arrival time = %.3Lfms, dropped\n", displaytime, p -> packetnumber, p -> tokensrequired, actualinterarrivaltime);
		//funlockfile(stdout);
		packetdropflag = 1;
		continue;
	}
	
	
	
	
	
	pthread_mutex_lock(&m);
	
	if(endpacketthreadcntc)
		{
			pthread_mutex_unlock(&m);
			return 0;
		}
	gettimeofday(&previousstart,0);
	addtopacketqueue(arrivalqueue,p);
	gettimeofday(&currenttime,0);
	
	displaytime = printtime();
	//flockfile(stdout);
    printf("%012.3Lfms: p%d enters Q1\n", displaytime, p-> packetnumber);
    //funlockfile(stdout);
    if(q2movement())
    {
		
		   displaytime = printtime();
		  
		 gettimeofday(&currenttime,0);
		 p->Q1departureTime = currenttime;
		 p->Q2arrivalTime = currenttime;
		 tockenbucketcount -= p->tokensrequired;
		 timeinq1 = (microseconds(p->Q1departureTime) - microseconds(p->Q1arrivalTime))/1000;
		 totaltimeinpacketqueue = totaltimeinpacketqueue + timeinq1;
		 
		// flockfile(stdout);
		 printf("%012.3Lfms: p%d leaves Q1, time in Q1 = %.3Lfms, token bucket now has %d tokens\n", displaytime,p->packetnumber,timeinq1,tockenbucketcount);
		
		 displaytime = printtime();
		 printf("%012.3Lfms: p%d enters Q2\n", displaytime, p -> packetnumber);
	     //funlockfile(stdout);
	     
	     if(!My402ListEmpty(arrivalqueue) && arrivalqueue!= NULL)
		  {
			 My402ListElem* first = My402ListFirst(arrivalqueue);  
			 My402ListAppend(departurequeue,first->obj);
			 My402ListUnlink(arrivalqueue,first);
			  
		  }
	     
		 pthread_cond_broadcast(&Q2empty);
	}

	
	else if(My402ListEmpty(departurequeue) && packetdropflag && tokendropflag && My402ListEmpty(arrivalqueue) && !tracefileflag )
	{
		 pthread_cond_broadcast(&Q2empty);
		 shutdownservers = 1;
	}
	
	if(endpacketthreadcntc)
		{
			pthread_mutex_unlock(&m);
			return 0;
		}
	pthread_mutex_unlock(&m);
	
	
   }
  return 0;	
}


void* processtoken(void* arg)
{
	int tokencount;
	long double sleeptime,displaytime;
	long double timeinq1;
	struct timeval currtime;
	double tokentime = 1000.0/(double)r;
	struct timeval previousfin,prevstart;
	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
	
	while(!controlc)
	{
		
		
	  gettimeofday(&previousfin,0);
	  
	  
	   if(My402ListEmpty(arrivalqueue) && endpacketthread)
	   {
		 endtokenthread = 1;
		 break;
		 return 0; 
	   }
	  	if (tokencount!=0)
	  	{
			sleeptime = tokentime*1000-(microseconds(previousfin) - microseconds(prevstart));
			usleep(sleeptime);
			if(controlc)
	        return 0 ;
		}
		if(tokencount==0)
		{
			usleep(tokentime*1000);
			if(controlc)
	        return 0 ;	
			
		}
		
		
		pthread_mutex_lock(&m);
		
		
		if(controlc)
		{
			pthread_mutex_unlock(&m);
			return 0;
		}
		
		gettimeofday(&prevstart,0);
		
		tokencount++;
		totaltokencount++;
		displaytime = printtime();
		if(tockenbucketcount+1 <=   B)
		{
			tockenbucketcount++;
			//flockfile(stdout);
			printf("%012.3Lfms: token t%d arrives, token bucket now has %d tokens\n", displaytime, tokencount, tockenbucketcount);
			//funlockfile(stdout);
			
		}
		else
		{
			//flockfile(stdout);
		    printf("%012.3Lfms: token t%d arrives, dropped\n", displaytime, tokencount);
		    //funlockfile(stdout);
			droptokenCount ++;	
			tokendropflag = 1;
			//continue;
		}
		
		if(q2movement())
		{
			
			My402ListElem *first = My402ListFirst(arrivalqueue);
			
			totaltimeinpacketqueue = totaltimeinpacketqueue + timeinq1;
		  	tockenbucketcount-= ((Packet*)(first->obj))->tokensrequired;
		  	displaytime = printtime();
		  	gettimeofday(&currtime,0);
		  	((Packet*)(first->obj))->Q1departureTime = currtime;
		  	((Packet*)(first->obj))->Q2arrivalTime = currtime;
		  	gettimeofday(&currtime,0);
			timeinq1 = ((microseconds(currtime))-(microseconds(((Packet*)(first -> obj))->Q1arrivalTime)))/1000;
		  	
		  	//flockfile(stdout);
		  	printf("%012.3Lfms: p%d leaves Q1, time in Q1 = %.3Lfms, token bucket now has %d tokens\n", displaytime,((Packet *)(first -> obj)) -> packetnumber,timeinq1,tockenbucketcount);
		  	//funlockfile(stdout);
		  	displaytime = printtime();
		  	//flockfile(stdout);
		  	
		  	printf("%012.3Lfms: p%d enters Q2\n", displaytime, ((Packet *)(first -> obj)) -> packetnumber);
		  	
		  	//funlockfile(stdout);
		  	if(departurequeue != NULL)
		  	{
		  	My402ListAppend(departurequeue,first->obj);
		  	My402ListUnlink(arrivalqueue,first);
		    }
		  	pthread_cond_broadcast(&Q2empty);
		}
		
	
		
	 if(My402ListEmpty(departurequeue) && packetdropflag && tokendropflag && My402ListEmpty(arrivalqueue) && !tracefileflag )
	{
		 pthread_cond_broadcast(&Q2empty);
		 shutdownservers = 1;
	}
	   if(controlc)
		{
			pthread_mutex_unlock(&m);
			return 0;
		}
		pthread_mutex_unlock(&m);
		
	}
	
	return 0;
}


void* processfirstserver(void* arg)
{
	long double sleeptime,displaytime,timeinq2,ServiceTime,TimeinSystem;
	struct timeval currtime;

	while(1)
	{	  
	   pthread_mutex_lock(&m);  
	  while(My402ListEmpty(departurequeue) || endserverthreads)	
	  { 	
		  if(controlc)
		  {
			   pthread_mutex_unlock(&m);
			    return 0; 
			 }
		  
		    if(flag)
		    {
			 pthread_cond_signal(&Q2empty); 
			 }  
			 if(shutdownservers)
			 { 
				 pthread_mutex_unlock(&m);
				return 0; 
			  }
		  
		  if(endtokenthread && My402ListEmpty(departurequeue))
	         { 
				 
				 pthread_mutex_unlock(&m);
				 return 0;
			     }
			     
		  pthread_cond_wait(&Q2empty,&m);
		  
	  }
	  
	 
	 if(controlc)
	 	 {
			pthread_mutex_unlock(&m);
			return 0; 
			 
			} 
	
		Packet* first = (Packet*)(My402ListFirst(departurequeue)->obj);
		sleeptime = first->servicetime;
		gettimeofday(&currtime,0);
		first->Q2departureTime = currtime;
		displaytime = printtime();
		timeinq2 = (microseconds(first->Q2departureTime)-microseconds(first->Q2arrivalTime))/1000.0;
		totaltimeinserverqueue = totaltimeinserverqueue + timeinq2;
		 
		 //endserver1 = 1;
		
	    //flockfile(stdout);
		printf("%012.3Lfms: p%d leaves Q2, time in Q2 = %.3Lfms\n", displaytime, first -> packetnumber,timeinq2);
		//funlockfile(stdout);
		displaytime = printtime();
		//flockfile(stdout);
		printf("%012.3Lfms: p%d begins service at S1, requesting %.3Lfms of service\n",displaytime,first->packetnumber,sleeptime);
		//funlockfile(stdout);
		My402ListUnlink(departurequeue,My402ListFirst(departurequeue));
	 	pthread_mutex_unlock(&m);
	 	
	 	
		
		usleep(sleeptime*1000);
		
	
		gettimeofday(&currtime,0);
	    pthread_mutex_lock(&m);
		
		ServiceTime = (microseconds(currtime)-microseconds(first->Q2departureTime))/1000;
		TimeinSystem = (microseconds(currtime)-microseconds(first->Q1arrivalTime))/1000;
		totaltimeinsystem+=TimeinSystem;
		totalsquaretime += TimeinSystem*TimeinSystem;
		displaytime = printtime();
		totaltimeins1 = totaltimeins1 + ServiceTime;
		totalservicetime+=ServiceTime;
		//flockfile(stdout);
		printf("%012.3Lfms: p%d departs from S1, service time = %.3Lfms, time in system = %.3Lfms\n", displaytime, first->packetnumber,ServiceTime,TimeinSystem);	
	    //funlockfile(stdout);
		//if(!My402ListEmpty(departurequeue)) 
		//My402ListUnlink(departurequeue,My402ListFirst(departurequeue));
	
	    //set a flag
		 flag = 1;      //to indicate that this server has served
		 
		 
	    pthread_mutex_unlock(&m);	    
	    if(controlc)
	    return 0;
	}	
	    return 0;
}


void* processsecondserver(void* arg)
{
	
	long double sleeptime,displaytime,timeinq2,ServiceTime,TimeinSystem;
	struct timeval currtime;
	
	while(1) {
		
	pthread_mutex_lock(&m);
   
	  while(My402ListEmpty(departurequeue) || endserverthreads)	
	  { 
		  if(controlc )
		  {
			 pthread_mutex_unlock(&m);
			 return 0; 
			 }
			 
			 
		  if(flag)
		  {
			 pthread_cond_signal(&Q2empty); 
			 }   
		     
		      if(shutdownservers)
			 {
				  pthread_mutex_unlock(&m);
				  return 0; 
			  }
		  if(endtokenthread && My402ListEmpty(departurequeue))
	         { 
				 pthread_mutex_unlock(&m);
				 return 0;
				 
			 }
		
	     				
		    pthread_cond_wait(&Q2empty,&m);
		 
	  }	
	  
	 	
	 	if(controlc)
	 	 {
			pthread_mutex_unlock(&m);
			return 0; 
			 
			}  
	    
		Packet* first = (Packet*)(My402ListFirst(departurequeue)->obj);
		sleeptime = first->servicetime;
		gettimeofday(&currtime,0);
		first->Q2departureTime = currtime;
		displaytime = printtime();
		timeinq2 = (microseconds(first->Q2departureTime)-microseconds(first->Q2arrivalTime))/1000.0;
		totaltimeinserverqueue = totaltimeinserverqueue + timeinq2;
	    
	   //flockfile(stdout);
		printf("%012.3Lfms: p%d leaves Q2, time in Q2 = %.3Lfms\n", displaytime, first -> packetnumber,timeinq2);
		//funlockfile(stdout);
		displaytime = printtime();
		//flockfile(stdout);
		printf("%012.3Lfms: p%d begins service at S2, requesting %.3Lfms of service\n",displaytime,first->packetnumber,sleeptime);
		//funlockfile(stdout);
		//endserver2 = 1;
		My402ListUnlink(departurequeue,My402ListFirst(departurequeue));
		
		pthread_mutex_unlock(&m);
		
		
		usleep(sleeptime*1000);
		
		
		gettimeofday(&currtime,0);
	
	    pthread_mutex_lock(&m);
		ServiceTime = (microseconds(currtime)-microseconds(first->Q2departureTime))/1000;
		totaltimeins2 = totaltimeins2 + ServiceTime;
		totalservicetime+=ServiceTime;
		TimeinSystem = (microseconds(currtime)-microseconds(first->Q1arrivalTime))/1000;
		totaltimeinsystem+=TimeinSystem;
		totalsquaretime += TimeinSystem*TimeinSystem;
		displaytime = printtime();
		
		//flockfile(stdout);
		printf("%012.3Lfms: p%d departs from S2, service time = %.3Lfms, time in system = %.3Lfms\n", displaytime, first->packetnumber,ServiceTime,TimeinSystem);	
		//funlockfile(stdout);
		//if(!My402ListEmpty(departurequeue)) 
		//My402ListUnlink(departurequeue,My402ListFirst(departurequeue));	
		//pthread_cond_signal(&Q2empty);
	    flag = 1;     					//to indicate that this server has served 
	    
	   
		pthread_mutex_unlock(&m);	
		
		 if(controlc)
	    return 0;
	    
		}
	
 return 0;	
}


int main(int argc, char *argv[]){
	
	readcommandline(argc,argv);
	displayinputparameters();
	initializequeues();
	sigemptyset(&set);
    sigaddset(&set, SIGINT);
	pthread_sigmask(SIG_BLOCK, &set, 0);
    
	 gettimeofday(&emulationstart,0);
	   long double displaytime;
	  
	   fprintf(stdout,"%012.3fms:emulation begins\n",0.00000000000000000000000);
	   
	   sigemptyset(&set);
       sigaddset(&set, SIGINT);
	   pthread_sigmask(SIG_BLOCK, &set, 0);
	   
	   pthread_create(&signalthread,0,processsignal,0);
       pthread_create(&packetarrivalthread,0,processpacket,0);
       pthread_create(&tokendepositingthread,0,processtoken,0);
       pthread_create(&firstserverthread,0,processfirstserver,0);
       pthread_create(&secondserverthread,0,processsecondserver,0);
       
       
       pthread_join(packetarrivalthread,0);
       pthread_join(tokendepositingthread,0);
       pthread_join(firstserverthread,0);
       pthread_join(secondserverthread,0);
       
       gettimeofday(&emulationend,0);
       displaytime = printtime();
       totalemulation = displaytime;
       fprintf(stdout,"%012.3Lfms:emulation ends\n",displaytime);
            
	   generatestatisctics();
	   return 0;
}
