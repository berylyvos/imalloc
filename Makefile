CFLAGS=-Wall -Wextra -Werror -std=c11 -pedantic -g

heap: main.c
	$(CC) $(CFLAGS) -o heap main.c