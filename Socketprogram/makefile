all:serverA serverB serverC serverD client

client:	client.o 
	gcc -o client -g client.o 

client.o: client.c 
	gcc -g -c -Wall client.c 
 
serverD: serverD.o 
	gcc -o serverD -g serverD.o 

serverD.o: serverD.c 
	gcc -g -c -Wall serverD.c 

serverC: serverC.o 
	gcc -o serverC -g serverC.o 

serverC.o: serverC.c 
	gcc -g -c -Wall serverC.c 

serverB: serverB.o 
	gcc -o serverB -g serverB.o  

serverB.o: serverB.c 
	gcc -g -c -Wall serverB.c

serverA: serverA.o 
	gcc -o serverA -g serverA.o 

serverA.o: serverA.c 
	gcc -g -c -Wall serverA.c



clean:
	rm -f *.o serverA serverB serverC client
