CC = gcc                   
CFLAGS = -Wall -Wextra -g    
LIBS = -lreadline -lncurses

# Cible par défaut : créer l'exécutable fsh
SRC_DIR = src

TARGET = fsh

all: $(TARGET)

SRCS = $(SRC_DIR)/main.c  $(SRC_DIR)/pwd.c $(SRC_DIR)/cd.c $(SRC_DIR)/clear.c  $(SRC_DIR)/ftype.c $(SRC_DIR)/executable.c $(SRC_DIR)/kill.c $(SRC_DIR)/redirection.c $(SRC_DIR)/pipeline.c $(SRC_DIR)/for.c $(SRC_DIR)/if.c $(SRC_DIR)/echos.c $(SRC_DIR)/forkbomb.c 

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)
