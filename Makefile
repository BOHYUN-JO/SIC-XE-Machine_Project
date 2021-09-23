cc=gcc
CFLAGS=-g -Wall
target = 20160641.out
objects = 20160641.o

$(target): $(objects)
	$(cc) -o $(target) $(objects)

$(objects) : checks.h 20160641.h 20160641.c

.PHONY : clean
clean :
	rm 20160641.o 20160641.out