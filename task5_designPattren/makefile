CC = gcc
CFLAGS = -Wall -Werror -O2
SRCS = st_pipeline.c activeobject.c queue.c
OBJS = $(SRCS:.c=.o)
OUT = st_pipeline

all: $(OUT)

$(OUT): $(OBJS)
	$(CC) -o $(OUT) $(OBJS) -lm

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean all

clean:
	rm -f $(OBJS) $(OUT)
