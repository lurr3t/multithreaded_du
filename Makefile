CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition
THREAD = -pthread
OUTPUT_FILE = bin/mdu
OBJ_DIR = bin

$(OUTPUT_FILE): $(OBJ_DIR)/mdu.o $(OBJ_DIR)/list.o $(OBJ_DIR)/t_queue.o $(OBJ_DIR)/error_handler.o
	$(CC) $(OBJ_DIR)/mdu.o $(OBJ_DIR)/list.o $(OBJ_DIR)/t_queue.o $(OBJ_DIR)/error_handler.o -o $(OUTPUT_FILE) $(THREAD)

$(OBJ_DIR)/mdu.o: mdu.c list.h t_queue.h error_handler.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c mdu.c -o $(OBJ_DIR)/mdu.o

$(OBJ_DIR)/t_queue.o: t_queue.c t_queue.h list.h error_handler.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c t_queue.c -o $(OBJ_DIR)/t_queue.o

$(OBJ_DIR)/list.o: list.c list.h error_handler.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c list.c -o $(OBJ_DIR)/list.o

$(OBJ_DIR)/error_handler.o: error_handler.c error_handler.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c error_handler.c -o $(OBJ_DIR)/error_handler.o

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
