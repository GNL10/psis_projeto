#makefile 

CC=gcc  #compiler
CFLAGS=-g -Wall -pthread -lSDL2 -lSDL2_ttf
CLIENT=memory_client
SERVER=memory_server
BOT=bot

all: compile_client compile_server compile_bot

compile_client:
	$(CC) memory_client.c UI_library.c board_library.c connections.c $(CFLAGS) -o $(CLIENT)
compile_server: 
	$(CC) memory_server.c board_library.c connections.c $(CFLAGS) -o $(SERVER)
compile_bot:
	$(CC) bot.c board_library.c connections.c $(CFLAGS) -o $(BOT)
clean:
	rm $(CLIENT) $(SERVER) $(BOT)
