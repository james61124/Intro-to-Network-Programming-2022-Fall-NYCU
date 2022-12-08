TARGET=client server 
CC=g++
CFLAGS= -Wall -Wextra -g
normal: $(TARGET)
client: client
	$(CC) client.cpp -o client
server: server
	$(CC) server.cpp -o server
clean:
	$(RM) $(TARGET)
