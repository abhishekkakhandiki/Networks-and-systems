# Soocket-Programming
This repository contains code related to client server architecture
/*A Readme file for Socket Programming*/


Summary:
This assignment implements Socket programming using TCP and UDP sockets. Each node in the system acts as a server and a client , able to send and receive packets.
The assignment deals with reading the network topology from the neighbouring nodes and calculating  the spanning tree for the topology . Prims algortihm is used to 
calculate the minimum cost associated with this network topology.

Code Files Included:
There are five .c files namely client.c,serverA.c,serverB.c,serverC.c,serverD.c,linkedlist.c  and two.h file utils.h,linkedlist.h that are included as part of submission.
Here is a brief overview of the working of all the files.

client.c - Acts as a server initially waiting to accept connections on TCP port 25580. After it receives connection from  all the clients , this file processes the data
received and forms an adjacancy matrix. Then, the client.c will open four UDP ports to communicate with  four servers A,B,C,D. This time client.c behaves as an actual 
client. Different port numbers used for this communication are 21580,22580,23580,24580 for servers A ,B ,C, D respectively. After broadcasting the information to the peers 
the client then calculates the minimum spanning tree using Prims algorithm and displays the output.

serverA.c - This file emulates as both a client and a server. Initially it acts as a client that reads a file provided and formats the data read according to a protocol
defined and sends the data via TCP port 25580. The file then manipulates itself as a server to be waiting for connection from client.c on the UDP port 22580 receiving data 
of the network topology. The serverA.c then displays the adjacancy matrix and the network topology.  

serverB.c - This file emulates as both a client and a server. Initially it acts as a client that reads a file provided and formats the data read according to a protocol
defined and sends the data via TCP port 25580. The file then manipulates itself as a server to be waiting for connection from client.c on the UDP port 23580 receiving data 
of the network topology. The serverB.c then displays the adjacancy matrix and the network topology. 

serverC.c - This file emulates as both a client and a server. Initially it acts as a client that reads a file provided and formats the data read according to a protocol
defined and sends the data via TCP port 25580. The file then manipulates itself as a server to be waiting for connection from client.c on the UDP port 24580 receiving data 
of the network topology. The serverC.c then displays the adjacancy matrix and the network topology. 

serverD.c - This file emulates as both a client and a server. Initially it acts as a client that reads a file provided and formats the data read according to a protocol
defined and sends the data via TCP port 25580. The file then manipulates itself as a server to be waiting for connection from client.c on the UDP port 25580 receiving data 
of the network topology. The serverD.c then displays the adjacancy matrix and the network topology. 

utils.h - This header file contains definitions of the port numbers used.

linkedlist.h - The header files that includes linked definitions

Running Program:

I have written a make file that directly compiles all the files and creates the executables. Just need to type "make" command , and all the source files are compiled 
and required executables are generated. The sequence of executables can be in any order.

Protocol  for Message Exchange

I have used "Server*" where * can be A,B,C,D as an identifier to distinguish the messages that arrive from different servers at the client. The distinguishing factor
between cost and neighbour is a "*"

Example- If server A has this information 

serverB 10
serverC 20

the message fromed will be 
serverA*serverB*10*serverC*20*

The client.c parses this message to obtain the actual neighbour information. The same technique is used on the client side to encode the data and the server parses it
using strtok function.

Idiosyncracy:
No idiosyncracy as such.

Reused code:
Have used Beejs code for socket programming , commented in client.c and serverA.c
Have used Prims algorithm from Google commented in client.c



