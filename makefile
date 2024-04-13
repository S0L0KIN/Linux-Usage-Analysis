#Makefile
CC = gcc
CCFLAGS = -Wall -Werror -Wextra

#Build
all: SystemUsage

SystemUsage : main.o display.o usage_functions.o pid_obj.o
	$(CC) $(CCFLAGS) -o $@ $^

main.o : main.c display.h usage_functions.h pid_obj.h
	$(CC) $(CCFLAGS) -c -o $@ $<

%.o : %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f *.o SystemUsage

.PHONY: help
help:
	@echo "	 Usage: make [command]"
	@echo "  all         Build SystemUsage"
	@echo "  clean       Remove compiled files"