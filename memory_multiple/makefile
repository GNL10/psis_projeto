#makefile 

CC=gcc  #compiler
CFLAGS=-g -lSDL2 -lSDL2_ttf
CLIENT=memory_client
SERVER=memory_server

all: compile_client compile_server

compile_client:
	$(CC) memory_client.c UI_library.c board_library.c $(CFLAGS) -o $(CLIENT)
compile_server: 
	$(CC) memory_server.c board_library.c $(CFLAGS) -o $(SERVER)
clean:
	rm $(CLIENT)
	rm $(SERVER)
