CC = gcc
CFLAGS = -Wall -g
OBJ = main.o client.o server.o chat.o
NAME = stnc

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJ) $(NAME)

