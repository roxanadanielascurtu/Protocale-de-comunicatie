CC=g++
CFLAGS=-I.

client: client.cpp helpers.cpp
		$(CC) -o client client.cpp helpers.cpp -Wall 
		
		
run: client
		./client

clean:
		rm -f *.o client
