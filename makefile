CC = gcc
SRC = $(wildcard *.c) $(wildcard *.h)
WARNINGS = -Wall -Wextra 
CFLAGS = $(WARNINGS) -march=native -O2 -flto -s -D_FORTIFY_SOURCE=1
DFLAGS = -g

build: $(SRC)
	$(CC) -o helish $(SRC) $(CFLAGS)

debug: $(SRC)
	$(CC) -o helish-debug $(SRC) $(DFLAGS)
