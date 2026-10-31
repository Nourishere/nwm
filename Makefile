all: nwm

SRC = nwm.c
H = nwm.h

CC=gcc
CC_FLAGS_COMMON=-Wall -Wextra -Wfatal-errors -march=native

DEBUG:=1

ifeq ($(DEBUG),1)
	CC_FLAGS=-DDEBUG -O0 -g $(CC_FLAGS_COMMON)
else ifeq ($(DEBUG), 0)
	CC_FLAGS=-DNDEBUG -O2 $(CC_FLAGS_COMMON)
else
	$(error Unknown DEBUG)
endif

LD_FLAGS=-lX11

indent:
	indent -linux $(SRC) $(H)
	rm -rf *.h~ *.c~

nwm: nwm.c
	$(CC) $(CC_FLAGS) $(LD_FLAGS) $^ -o $@

clean:
	rm -rf nwm

.PHONY:
	clean all
