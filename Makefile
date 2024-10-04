CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition
THREAD = -pthread
OUTPUT_FILE = mdu

$(OUTPUT_FILE): mdu.o list.o t_queue.o error_handler.o
	$(CC) mdu.o list.o t_queue.o error_handler.o -o $(OUTPUT_FILE) $(THREAD)

mdu.o: mdu.c list.h t_queue.h error_handler.h
	$(CC) $(CFLAGS) -c mdu.c

t_queue.o: t_queue.c t_queue.h list.h error_handler.h
	$(CC) $(CFLAGS) -c t_queue.c

list.o: list.c list.h error_handler.h
	$(CC) $(CFLAGS) -c list.c

error_handler.o: error_handler.c error_handler.h
	$(CC) $(CFLAGS) -c error_handler.c

clean:
	rm *.o mdu
