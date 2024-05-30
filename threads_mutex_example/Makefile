.PHONY: all
all: problem mutex

problem:problem.c
	gcc -o problem problem.c -lpthread

mutex:  mutex.c
	gcc -o mutex mutex.c -lpthread

.PHONY: clean
clean:
	-rm problem mutex 2>/dev/null
