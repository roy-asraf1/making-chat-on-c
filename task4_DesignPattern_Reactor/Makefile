FLAGS = -Wall -g
CC = gcc


all: react_server 



# client.o: client.c 
# 	$(CC) $(FLAGS) -c client.c 

# client: client.o
# 	$(CC) $(FLAGS) -o client client.o

react_server.o: react_server.c 
	$(CC) $(FLAGS) -c react_server.c 

react_server: react_server.o
	$(CC) $(FLAGS) -o react_server react_server.o



.PHONEY: clean
clean:
	rm -f *.o *.a *.so react_server 