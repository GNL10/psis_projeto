#makefile 

CC=gcc  #compiler
TARGET=memory_game  #target file name
 
all:
	$(CC) memory-single.c UI_library.c board_library.c -g -lSDL2 -lSDL2_ttf -o $(TARGET)
#gcc memory-single.c UI_library.c board_library.c -lSDL2 -lSDL2_ttf -o teste
 
clean:
	rm $(TARGET)
