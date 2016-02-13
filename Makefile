BIN_DIR = bin
SRC_DIR = src
OBJ_DIR = obj
CC = gcc
CFLAGS = -Wall -g -O2 -pedantic -std=c99

all: setup shared msock server client

setup:
	mkdir -p $(BIN_DIR) $(OBJ_DIR)

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

shared:
	$(CC) $(CFLAGS) -c $(SRC_DIR)/shared.c -o $(OBJ_DIR)/shared.o

msock:
	$(CC) $(CFLAGS) -c $(SRC_DIR)/msock.c -o $(OBJ_DIR)/msock.o

server: server.o
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server $(OBJ_DIR)/server.o $(OBJ_DIR)/shared.o $(OBJ_DIR)/msock.o

client: client.o
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client $(OBJ_DIR)/client.o $(OBJ_DIR)/shared.o $(OBJ_DIR)/msock.o

server.o: $(SRC_DIR)/server.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/server.c -o $(OBJ_DIR)/server.o

client.o: $(SRC_DIR)/client.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/client.c -o $(OBJ_DIR)/client.o
